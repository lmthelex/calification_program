#include "../headers/Rubric.hpp"

namespace
{
string trim(const string &value)
{
    const auto first = find_if_not(value.begin(), value.end(), [](unsigned char c)
    {
        return isspace(c);
    });
    const auto last = find_if_not(value.rbegin(), value.rend(), [](unsigned char c)
    {
        return isspace(c);
    }).base();

    if (first >= last)
    {
        return "";
    }
    return string(first, last);
}

string normalize_id(string id)
{
    id = trim(id);
    while (!id.empty() and (id.back() == '.' or id.back() == ';'))
    {
        id.pop_back();
    }
    return id;
}

double parse_number(const string &text, const string &context)
{
    const string value = trim(text);
    size_t read = 0;
    double number = 0.0;

    try
    {
        number = stod(value, &read);
    }
    catch (const exception &)
    {
        throw runtime_error("Invalid number in " + context + ": \"" + value + "\"");
    }

    if (read != value.size() or !isfinite(number))
    {
        throw runtime_error("Invalid number in " + context + ": \"" + value + "\"");
    }
    return number;
}

double parse_bracketed_score(string score, const string &context)
{
    score = trim(score);
    if (score.size() < 3 or score.front() != '[' or score.back() != ']')
    {
        throw runtime_error("Expected a score in brackets in " + context);
    }
    return parse_number(score.substr(1, score.size() - 2), context);
}

struct RubricLine
{
    string id;
    double base_score;
    string description;
};

RubricLine parse_rubric_line(const string &line, size_t line_number)
{
    const string context = "rubric line " + to_string(line_number);
    const auto first_separator = line.find(';');

    if (first_separator != string::npos)
    {
        const auto second_separator = line.find(';', first_separator + 1);
        if (second_separator == string::npos)
        {
            throw runtime_error("Malformed semicolon-delimited " + context);
        }

        const string id = normalize_id(line.substr(0, first_separator));
        const double score = parse_bracketed_score(
                line.substr(first_separator + 1,
                            second_separator - first_separator - 1), context);
        const string description = trim(line.substr(second_separator + 1));
        if (id.empty() or description.empty())
        {
            throw runtime_error("Missing id or description in " + context);
        }
        return {id, score, description};
    }

    const auto open_bracket = line.find('[');
    const auto close_bracket = line.find(']', open_bracket);
    if (open_bracket == string::npos or close_bracket == string::npos)
    {
        throw runtime_error("Malformed " + context);
    }

    const string id = normalize_id(line.substr(0, open_bracket));
    const double score = parse_bracketed_score(
            line.substr(open_bracket, close_bracket - open_bracket + 1), context);
    const string description = trim(line.substr(close_bracket + 1));
    if (id.empty() or description.empty())
    {
        throw runtime_error("Missing id or description in " + context);
    }
    return {id, score, description};
}

vector<string> split_scored_raw_line(const string &line, size_t line_number)
{
    vector<string> fields;
    size_t begin = 0;

    for (int separator = 0; separator < 3; ++separator)
    {
        const auto position = line.find(';', begin);
        if (position == string::npos)
        {
            throw runtime_error("Malformed raw_note.txt line " +
                                to_string(line_number));
        }
        fields.push_back(trim(line.substr(begin, position - begin)));
        begin = position + 1;
    }
    fields.push_back(trim(line.substr(begin)));
    return fields;
}

string display_id(const string &id)
{
    return id + ".";
}
}

const string &Rubric::get_student_name() const
{
    return student_name;
}

void Rubric::set_student_name(string student_name_)
{
    student_name = std::move(student_name_);
}

double Rubric::get_base_score() const
{
    double total = 0.0;
    for (const auto &criterion: criteria)
    {
        total += criterion.get_base_score();
    }
    return total;
}

double Rubric::get_achieved_score() const
{
    double total = 0.0;
    for (const auto &criterion: criteria)
    {
        total += criterion.get_achieved_score();
    }
    for (const auto &deduction: deductions)
    {
        total += deduction.get_achieved_deduct_score();
    }
    return total;
}

const vector<Criterion> &Rubric::get_criteria() const
{
    return criteria;
}

const vector<Deduction> &Rubric::get_deductions() const
{
    return deductions;
}

const vector<Observation> &Rubric::get_observations() const
{
    return observations;
}

Criterion *Rubric::find_criterion(const string &id)
{
    const string wanted = normalize_id(id);
    for (auto &criterion: criteria)
    {
        if (criterion.get_id() == wanted)
        {
            return &criterion;
        }
    }
    return nullptr;
}

Deduction *Rubric::find_deduction(const string &id)
{
    const string wanted = normalize_id(id);
    for (auto &deduction: deductions)
    {
        if (deduction.get_id() == wanted)
        {
            return &deduction;
        }
    }
    return nullptr;
}

bool Rubric::has_any_calification() const
{
    const bool has_criterion = any_of(criteria.begin(), criteria.end(),
                                      [](const Criterion &criterion)
    {
        return criterion.has_achieved_score();
    });
    const bool has_deduction = any_of(deductions.begin(), deductions.end(),
                                      [](const Deduction &deduction)
    {
        return deduction.has_achieved_deduct_score();
    });
    return has_criterion or has_deduction or !observations.empty();
}

bool Rubric::is_ready() const
{
    return !criteria.empty() and all_of(criteria.begin(), criteria.end(),
                                        [](const Criterion &criterion)
    {
        return criterion.has_achieved_score();
    });
}

void Rubric::read_rubric(istream &rubric_file)
{
    criteria.clear();
    deductions.clear();
    observations.clear();
    student_name.clear();

    enum class Section { NONE, CRITERIA, DEDUCTIONS, OBSERVATIONS };
    Section section = Section::NONE;
    bool found_criteria_section = false;
    bool found_deductions_section = false;
    unordered_set<string> criterion_ids;
    unordered_set<string> deduction_ids;
    string line;
    size_t line_number = 0;

    while (getline(rubric_file, line))
    {
        ++line_number;
        if (!line.empty() and line.back() == '\r')
        {
            line.pop_back();
        }
        if (line_number == 1 and line.rfind("\xEF\xBB\xBF", 0) == 0)
        {
            line.erase(0, 3);
        }
        line = trim(line);
        if (line.empty() or line == "-")
        {
            continue;
        }
        if (line == "Puntaje")
        {
            section = Section::CRITERIA;
            found_criteria_section = true;
            continue;
        }
        if (line == "Descuentos")
        {
            section = Section::DEDUCTIONS;
            found_deductions_section = true;
            continue;
        }
        if (line == "Observaciones")
        {
            section = Section::OBSERVATIONS;
            continue;
        }
        if (section == Section::NONE)
        {
            throw runtime_error("Unexpected content on rubric line " +
                                to_string(line_number));
        }
        if (section == Section::OBSERVATIONS)
        {
            continue;
        }

        RubricLine item = parse_rubric_line(line, line_number);
        if (section == Section::CRITERIA)
        {
            if (item.base_score < 0.0)
            {
                throw runtime_error("Criterion " + item.id +
                                    " cannot have a negative base score");
            }
            if (!criterion_ids.insert(item.id).second)
            {
                throw runtime_error("Duplicate criterion id: " + item.id);
            }
            criteria.emplace_back(item.id, item.description, item.base_score);
        }
        else
        {
            if (item.base_score > 0.0)
            {
                throw runtime_error("Deduction " + item.id +
                                    " cannot have a positive base score");
            }
            if (!deduction_ids.insert(item.id).second)
            {
                throw runtime_error("Duplicate deduction id: " + item.id);
            }
            deductions.emplace_back(item.id, item.description, item.base_score);
        }
    }

    if (!rubric_file.eof() or !found_criteria_section or
        !found_deductions_section or criteria.empty())
    {
        throw runtime_error("The rubric is incomplete or could not be read");
    }
    if (abs(get_base_score() - 20.0) > 0.0001)
    {
        ostringstream message;
        message << fixed << setprecision(2)
                << "Rubric criteria total must be 20.00, but is "
                << get_base_score();
        throw runtime_error(message.str());
    }
}

void Rubric::read_raw_note(istream &raw_note_file)
{
    for (auto &criterion: criteria)
    {
        criterion.clear_achieved_score();
    }
    for (auto &deduction: deductions)
    {
        deduction.clear_achieved_deduct_score();
    }
    observations.clear();

    enum class Section { NONE, CRITERIA, DEDUCTIONS, OBSERVATIONS };
    Section section = Section::NONE;
    unordered_set<string> read_criteria;
    unordered_set<string> read_deductions;
    string line;
    size_t line_number = 0;

    while (getline(raw_note_file, line))
    {
        ++line_number;
        if (!line.empty() and line.back() == '\r')
        {
            line.pop_back();
        }
        const string stripped = trim(line);
        if (stripped.empty())
        {
            continue;
        }
        if (stripped == "Puntaje")
        {
            section = Section::CRITERIA;
            continue;
        }
        if (stripped == "Descuentos")
        {
            section = Section::DEDUCTIONS;
            continue;
        }
        if (stripped == "Observaciones")
        {
            section = Section::OBSERVATIONS;
            continue;
        }
        if (section == Section::NONE)
        {
            throw runtime_error("Unexpected content in raw_note.txt line " +
                                to_string(line_number));
        }

        if (section == Section::OBSERVATIONS)
        {
            const auto separator = line.find(';');
            if (separator == string::npos)
            {
                observations.emplace_back("", stripped);
            }
            else
            {
                observations.emplace_back(
                        normalize_id(line.substr(0, separator)),
                        trim(line.substr(separator + 1)));
            }
            continue;
        }

        const vector<string> fields = split_scored_raw_line(line, line_number);
        const string id = normalize_id(fields[0]);
        const double raw_base = parse_bracketed_score(
                fields[1], "raw_note.txt line " + to_string(line_number));
        if (section == Section::CRITERIA)
        {
            Criterion *criterion = find_criterion(id);
            if (criterion == nullptr)
            {
                throw runtime_error("Unknown criterion in raw_note.txt: " + id);
            }
            if (!read_criteria.insert(id).second)
            {
                throw runtime_error("Duplicate criterion in raw_note.txt: " + id);
            }
            if (abs(raw_base - criterion->get_base_score()) > 0.0001)
            {
                throw runtime_error("Base score changed for criterion " + id);
            }
            if (!fields[2].empty())
            {
                criterion->set_achieved_score(parse_number(
                        fields[2], "criterion " + id + " in raw_note.txt"));
            }
        }
        else
        {
            Deduction *deduction = find_deduction(id);
            if (deduction == nullptr)
            {
                throw runtime_error("Unknown deduction in raw_note.txt: " + id);
            }
            if (!read_deductions.insert(id).second)
            {
                throw runtime_error("Duplicate deduction in raw_note.txt: " + id);
            }
            if (abs(raw_base - deduction->get_base_deduct_score()) > 0.0001)
            {
                throw runtime_error("Base score changed for deduction " + id);
            }
            if (!fields[2].empty())
            {
                deduction->set_achieved_deduct_score(parse_number(
                        fields[2], "deduction " + id + " in raw_note.txt"));
            }
        }
    }

    if (!raw_note_file.eof())
    {
        throw runtime_error("Could not read raw_note.txt");
    }
}

void Rubric::write_raw_note(ostream &raw_note_file) const
{
    raw_note_file << "Puntaje\n";
    for (const auto &criterion: criteria)
    {
        raw_note_file << criterion.get_id() << ";[" << fixed << setprecision(2)
                      << criterion.get_base_score() << "];";
        if (criterion.has_achieved_score())
        {
            raw_note_file << fixed << setprecision(2)
                          << criterion.get_achieved_score();
        }
        raw_note_file << ";" << criterion.get_description() << "\n";
    }

    raw_note_file << "Descuentos\n";
    for (const auto &deduction: deductions)
    {
        raw_note_file << deduction.get_id() << ";[" << fixed << setprecision(2)
                      << deduction.get_base_deduct_score() << "];";
        if (deduction.has_achieved_deduct_score())
        {
            raw_note_file << fixed << setprecision(2)
                          << deduction.get_achieved_deduct_score();
        }
        raw_note_file << ";" << deduction.get_description() << "\n";
    }

    raw_note_file << "Observaciones\n";
    for (const auto &observation: observations)
    {
        raw_note_file << observation.get_id() << ";"
                      << observation.get_description() << "\n";
    }
}

void Rubric::print(ostream &output) const
{
    for (const auto &criterion: criteria)
    {
        output << display_id(criterion.get_id()) << " [" << fixed
               << setprecision(2) << criterion.get_base_score() << "]"
               << criterion.get_description() << "\n";
    }
    output << "\nTotal: " << fixed << setprecision(2) << get_base_score() << "\n";
}

void Rubric::print_feedback(ostream &output) const
{
    output << "======================== RUBRICA ========================\n";
    output << "Alumno: " << student_name << "\n\n";
    output << "CRITERIOS:\n" << string(100, '=') << "\n";
    output << left << setw(8) << "ID" << right << setw(10) << "Base"
           << setw(12) << "Obtenido" << string(5, ' ')
           << left << "Descripcion\n" << string(100, '-') << "\n";

    double criteria_total = 0.0;
    for (const auto &criterion: criteria)
    {
        criterion.print(output);
        criteria_total += criterion.get_achieved_score();
    }
    output << string(100, '-') << "\n";
    output << "Puntaje obtenido: " << fixed << setprecision(2)
           << criteria_total << "\n\n";

    output << "DESCUENTOS:\n" << string(100, '=') << "\n";
    output << left << setw(8) << "ID" << right << setw(10) << "Base"
           << setw(12) << "Descontado" << string(5, ' ')
           << left << "Descripcion\n" << string(100, '-') << "\n";

    double deduction_total = 0.0;
    for (const auto &deduction: deductions)
    {
        deduction.print(output);
        deduction_total += deduction.get_achieved_deduct_score();
    }
    output << string(100, '-') << "\n";
    output << "Descuentos obtenidos: " << fixed << setprecision(2)
           << deduction_total << "\n\n";

    output << "OBSERVACIONES:\n" << string(100, '=') << "\n";
    for (const auto &observation: observations)
    {
        observation.print(output);
    }
    output << string(100, '-') << "\n\n";
    output << "NOTA FINAL: " << fixed << setprecision(2)
           << get_achieved_score() << "\n";
    output << "========================================================\n";
}

void Rubric::add_observation(const Observation &observation)
{
    const bool already_registered = any_of(
            observations.begin(), observations.end(),
            [&observation](const Observation &current)
    {
        if (!observation.get_id().empty())
        {
            return current.get_id() == observation.get_id();
        }
        return current.get_id().empty() and
               current.get_description() == observation.get_description();
    });
    if (!already_registered)
    {
        observations.push_back(observation);
    }
}

void Rubric::refresh_observations(
        const vector<Observation> &general_observations)
{
    for (auto &registered: observations)
    {
        if (registered.get_id().empty())
        {
            continue;
        }
        const auto found = find_if(
                general_observations.begin(), general_observations.end(),
                [&registered](const Observation &general)
        {
            return general.get_id() == registered.get_id();
        });
        if (found != general_observations.end())
        {
            registered.set_description(found->get_description());
        }
    }
}
