#include "../headers/CalificationProgram.hpp"

#include <cstdio>
#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif

namespace
{
bool use_terminal_colors()
{
    if (getenv("NO_COLOR") != nullptr)
    {
        return false;
    }
    const char *force_color = getenv("CLICOLOR_FORCE");
    if (force_color != nullptr and string(force_color) != "0")
    {
        return true;
    }
#if defined(_WIN32)
    return _isatty(_fileno(stdout)) != 0;
#else
    return isatty(fileno(stdout)) != 0;
#endif
}

const char *pink()
{
    return use_terminal_colors() ? "\033[38;5;205m" : "";
}

const char *soft_pink()
{
    return use_terminal_colors() ? "\033[38;5;218m" : "";
}

const char *bold_pink()
{
    return use_terminal_colors() ? "\033[1;38;5;205m" : "";
}

const vector<string> &list_line_colors()
{
    static const vector<string> colors = {
            "\033[38;2;218;112;163m",
            "\033[38;2;225;120;171m",
            "\033[38;2;232;128;179m",
            "\033[38;2;239;136;187m",
            "\033[38;2;246;144;195m",
            "\033[38;2;239;136;187m",
            "\033[38;2;232;128;179m",
            "\033[38;2;225;120;171m"
    };
    static const vector<string> no_colors;
    return use_terminal_colors() ? colors : no_colors;
}

const char *list_item_color(size_t index)
{
    const vector<string> &colors = list_line_colors();
    return colors.empty() ? "" : colors[index % colors.size()].c_str();
}

const char *achieved_score_color()
{
    return use_terminal_colors() ? "\033[1;38;5;84m" : "";
}

const char *base_score_color()
{
    return use_terminal_colors() ? "\033[38;5;147m" : "";
}

const char *student_state_color(StudentState state)
{
    if (!use_terminal_colors())
    {
        return "";
    }
    switch (state)
    {
        case StudentState::READY:
            return "\033[1;38;5;84m";
        case StudentState::INCOMPLETE:
            return "\033[1;38;5;220m";
        case StudentState::BLANK:
            return "\033[1;38;5;117m";
    }
    return "";
}

const char *reset_color()
{
    return use_terminal_colors() ? "\033[0m" : "";
}

void print_section_title(const string &title)
{
    cout << bold_pink() << "\n  \u2726 " << title << " \u2726\n"
         << soft_pink() << "  " << string(title.size() + 4, '-') << "\n"
         << reset_color();
}

void print_student_card(const Student &student, const string &title)
{
    print_section_title(title);
    cout << pink() << "  Name  " << reset_color()
         << student.get_last_name() << " " << student.get_first_name()
         << "\n";
    cout << pink() << "  Code  " << reset_color()
         << student.get_code() << "\n";
    cout << pink() << "  State " << reset_color()
         << student_state_color(student.get_state())
         << student.get_state_name() << reset_color() << "\n";
    cout << pink() << "  Path  " << reset_color();
    if (student.get_project_folder())
    {
        cout << student.get_project_folder()->string();
    }
    else
    {
        cout << "-";
    }
    cout << "\n";
}

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
    return first < last ? string(first, last) : "";
}

string normalize_observation_id(string id)
{
    id = trim(id);
    if (!id.empty())
    {
        id[0] = static_cast<char>(toupper(static_cast<unsigned char>(id[0])));
    }
    while (!id.empty() and id.back() == '.')
    {
        id.pop_back();
    }
    return id;
}

bool is_observation_id(const string &value)
{
    if (value.size() < 2 or value.front() != 'O')
    {
        return false;
    }
    return all_of(value.begin() + 1, value.end(), [](unsigned char c)
    {
        return isdigit(c);
    });
}

double parse_input_number(const string &input)
{
    const string value = trim(input);
    size_t read = 0;
    double number = 0.0;
    try
    {
        number = stod(value, &read);
    }
    catch (const exception &)
    {
        throw invalid_argument("The score must be a number");
    }
    if (read != value.size() or !isfinite(number))
    {
        throw invalid_argument("The score must be a number");
    }
    return number;
}

vector<Observation>::const_iterator find_observation(
        const vector<Observation> &observations, const string &id)
{
    return find_if(observations.begin(), observations.end(),
                   [&id](const Observation &observation)
    {
        return observation.get_id() == id;
    });
}

string normalize_rubric_id(string id)
{
    id = trim(id);
    while (!id.empty() and id.back() == '.')
    {
        id.pop_back();
    }
    return id;
}

string bracketed_score(double score)
{
    ostringstream output;
    output << "[" << fixed << setprecision(2) << score << "]";
    return output.str();
}

string command_name(const string &option)
{
    istringstream input(option);
    string command;
    input >> command;
    return command;
}

size_t criteria_display_width(const string &option, size_t default_width)
{
    istringstream input(option);
    string command;
    string width_text;
    string extra;
    input >> command;
    if (command != "sc")
    {
        throw invalid_argument("Usage: sc [width]");
    }
    if (!(input >> width_text))
    {
        return default_width;
    }
    if (input >> extra)
    {
        throw invalid_argument("Usage: sc [width]");
    }

    size_t read = 0;
    unsigned long width = 0;
    try
    {
        width = stoul(width_text, &read);
    }
    catch (const exception &)
    {
        throw invalid_argument("Criteria width must be an integer from 30 to 300");
    }
    if (read != width_text.size() or width < 30 or width > 300)
    {
        throw invalid_argument("Criteria width must be an integer from 30 to 300");
    }
    return static_cast<size_t>(width);
}
}

CalificationProgram::CalificationProgram(string evaluation_name_, path lab_folder_,
                                         size_t report_width_)
    : evaluation_name(std::move(evaluation_name_))
    , lab_folder(std::move(lab_folder_))
    , students_loaded(false)
    , report_width(report_width_)
{
    load_rubric();
    synchronize_raw_notes();
}

void CalificationProgram::load_rubric()
{
    if (!is_directory(lab_folder))
    {
        throw runtime_error("Evaluation folder does not exist: " +
                            lab_folder.string());
    }

    const path rubric_path = lab_folder / "meta" / "rubric.txt";
    ifstream input(rubric_path);
    if (!input.is_open())
    {
        throw runtime_error("Could not open " + rubric_path.string());
    }
    rubric.read_rubric(input);
}

void CalificationProgram::synchronize_raw_notes()
{
    const path projects_folder = lab_folder / "projects";
    if (!is_directory(projects_folder))
    {
        return;
    }

    for (const auto &entry: recursive_directory_iterator(
            projects_folder, directory_options::skip_permission_denied))
    {
        if (entry.is_regular_file() and entry.path().filename() == "raw_note.txt")
        {
            synchronize_raw_note(entry.path());
        }
    }
}

void CalificationProgram::synchronize_raw_note(const path &raw_note_path) const
{
    ifstream input(raw_note_path, ios::binary);
    if (!input.is_open())
    {
        throw runtime_error("Could not open " + raw_note_path.string());
    }
    ostringstream input_buffer;
    input_buffer << input.rdbuf();
    if (!input.eof() and input.fail())
    {
        throw runtime_error("Could not read " + raw_note_path.string());
    }
    const string original = input_buffer.str();
    string synchronized;
    synchronized.reserve(original.size());

    enum class Section { NONE, CRITERIA, DEDUCTIONS, OBSERVATIONS };
    Section section = Section::NONE;
    size_t line_begin = 0;

    while (line_begin < original.size())
    {
        const size_t newline = original.find('\n', line_begin);
        const bool has_newline = newline != string::npos;
        const size_t line_end = has_newline ? newline : original.size();
        string line = original.substr(line_begin, line_end - line_begin);
        string comparable = line;
        if (!comparable.empty() and comparable.back() == '\r')
        {
            comparable.pop_back();
        }
        comparable = trim(comparable);

        if (comparable == "Puntaje")
        {
            section = Section::CRITERIA;
        }
        else if (comparable == "Descuentos")
        {
            section = Section::DEDUCTIONS;
        }
        else if (comparable == "Observaciones")
        {
            section = Section::OBSERVATIONS;
        }
        else if (section == Section::CRITERIA or section == Section::DEDUCTIONS)
        {
            const size_t first_separator = line.find(';');
            const size_t second_separator = first_separator == string::npos
                    ? string::npos : line.find(';', first_separator + 1);
            if (second_separator != string::npos)
            {
                const string id = normalize_rubric_id(
                        line.substr(0, first_separator));
                optional<double> base_score;

                if (section == Section::CRITERIA)
                {
                    optional<int> numeric_id;
                    try
                    {
                        numeric_id = Criterion::parse_id(id);
                    }
                    catch (const invalid_argument &)
                    {
                        numeric_id.reset();
                    }
                    if (numeric_id)
                    {
                        const auto found = find_if(
                                rubric.get_criteria().begin(),
                                rubric.get_criteria().end(),
                                [&numeric_id](const Criterion &criterion)
                        {
                            return criterion.get_numeric_id() == *numeric_id;
                        });
                        if (found != rubric.get_criteria().end())
                        {
                            base_score = found->get_base_score();
                        }
                    }
                }
                else
                {
                    const auto found = find_if(
                            rubric.get_deductions().begin(),
                            rubric.get_deductions().end(),
                            [&id](const Deduction &deduction)
                    {
                        return deduction.get_id() == id;
                    });
                    if (found != rubric.get_deductions().end())
                    {
                        base_score = found->get_base_deduct_score();
                    }
                }

                if (base_score)
                {
                    line = line.substr(0, first_separator + 1) +
                           bracketed_score(*base_score) +
                           line.substr(second_separator);
                }
            }
        }

        synchronized += line;
        if (has_newline)
        {
            synchronized += '\n';
            line_begin = newline + 1;
        }
        else
        {
            line_begin = original.size();
        }
    }

    if (synchronized == original)
    {
        return;
    }

    const path temporary = raw_note_path.string() + ".sync.tmp";
    ofstream output(temporary, ios::binary | ios::trunc);
    if (!output.is_open())
    {
        throw runtime_error("Could not write " + temporary.string());
    }
    output << synchronized;
    output.close();
    if (!output)
    {
        throw runtime_error("Could not finish writing " + temporary.string());
    }

    error_code error;
    rename(temporary, raw_note_path, error);
    if (error)
    {
        throw runtime_error("Could not replace " + raw_note_path.string() +
                            ": " + error.message());
    }
}

void CalificationProgram::load_students()
{
    const path students_path = lab_folder / "meta" / "students.txt";
    ifstream input(students_path);
    if (!input.is_open())
    {
        throw runtime_error("Could not open " + students_path.string());
    }

    vector<Student> loaded_students = Student::read_students(input, rubric);
    for (auto &student: loaded_students)
    {
        student.prepare(lab_folder);
    }
    students = std::move(loaded_students);
    students_loaded = true;
}

void CalificationProgram::ensure_students_loaded()
{
    if (!students_loaded)
    {
        load_students();
    }
}

vector<Observation> CalificationProgram::load_general_observations() const
{
    const path observations_path = lab_folder / "meta" / "observations.txt";
    ifstream input(observations_path);
    if (!input.is_open())
    {
        throw runtime_error("Could not open " + observations_path.string());
    }

    vector<Observation> observations;
    unordered_set<string> ids;
    string line;
    size_t line_number = 0;
    while (getline(input, line))
    {
        ++line_number;
        if (!line.empty() and line.back() == '\r')
        {
            line.pop_back();
        }
        line = trim(line);
        if (line.empty() or line.front() == '#')
        {
            continue;
        }

        string id;
        string description;
        const auto separator = line.find(';');
        if (separator != string::npos)
        {
            id = normalize_observation_id(line.substr(0, separator));
            description = trim(line.substr(separator + 1));
        }
        else
        {
            const auto point = line.find('.');
            if (point == string::npos)
            {
                throw runtime_error("Malformed observations.txt line " +
                                    to_string(line_number));
            }
            id = normalize_observation_id(line.substr(0, point));
            description = trim(line.substr(point + 1));
        }

        if (!is_observation_id(id) or description.empty())
        {
            throw runtime_error("Malformed observations.txt line " +
                                to_string(line_number));
        }
        if (!ids.insert(id).second)
        {
            throw runtime_error("Duplicate observation id: " + id);
        }
        observations.emplace_back(id, description);
    }
    if (!input.eof())
    {
        throw runtime_error("Could not read " + observations_path.string());
    }
    return observations;
}

Observation CalificationProgram::create_general_observation(
        const string &description) const
{
    vector<Observation> observations = load_general_observations();
    unsigned long next_number = 1;
    for (const auto &observation: observations)
    {
        const string id = observation.get_id();
        try
        {
            next_number = max(next_number, stoul(id.substr(1)) + 1);
        }
        catch (const exception &)
        {
            throw runtime_error("Invalid observation id: " + id);
        }
    }

    Observation created("O" + to_string(next_number), description);
    observations.push_back(created);

    const path destination = lab_folder / "meta" / "observations.txt";
    const path temporary = lab_folder / "meta" / "observations.txt.tmp";
    ofstream output(temporary, ios::trunc);
    if (!output.is_open())
    {
        throw runtime_error("Could not write " + temporary.string());
    }
    for (const auto &observation: observations)
    {
        output << observation.get_id() << ";"
               << observation.get_description() << "\n";
    }
    output.close();
    if (!output)
    {
        throw runtime_error("Could not finish writing " + temporary.string());
    }

    error_code error;
    rename(temporary, destination, error);
    if (error)
    {
        throw runtime_error("Could not replace " + destination.string() +
                            ": " + error.message());
    }
    return created;
}

void CalificationProgram::print_welcome() const
{
    cout << bold_pink()
         << "\n  \u2665 CALIFICATION PROGRAM \u2665\n"
         << soft_pink() << "  -------------------------\n"
         << reset_color();
    cout << pink() << "  Evaluation  " << reset_color()
         << evaluation_name << "\n";
    cout << pink() << "  Rubric      " << reset_color() << "Loaded\n\n";
    cout << soft_pink();
    rubric.print(cout, report_width, list_line_colors(), soft_pink());
    cout << reset_color();
}

void CalificationProgram::print_menu() const
{
    print_section_title("MAIN MENU");
    cout << list_item_color(0) << "  [a]  Show students" << reset_color()
         << "\n";
    cout << list_item_color(1) << "  [b]  Grade students" << reset_color()
         << "\n";
    cout << list_item_color(2) << "  [c]  Write feedback" << reset_color()
         << "\n";
    cout << list_item_color(3) << "  [d]  Send emails" << reset_color()
         << "\n";
    cout << list_item_color(4) << "  [q]  Quit" << reset_color() << "\n";
    cout << bold_pink() << "\n  Choose an option \u203a " << reset_color()
         << flush;
}

void CalificationProgram::show_students()
{
    load_students();

    print_section_title("STUDENTS");
    cout << bold_pink() << left << setw(12) << "Code"
         << setw(24) << "Last_name"
         << setw(24) << "First_name"
         << setw(10) << "Project"
         << "State\n" << reset_color();

    size_t without_project = 0;
    for (size_t index = 0; index < students.size(); ++index)
    {
        const Student &student = students[index];
        if (!student.has_project())
        {
            ++without_project;
        }
        cout << list_item_color(index)
             << left << setw(12) << student.get_code()
             << setw(24) << student.get_last_name()
             << setw(24) << student.get_first_name()
             << setw(10) << (student.has_project() ? "true" : "false");
        cout << student_state_color(student.get_state())
             << student.get_state_name() << reset_color() << "\n";
    }
    cout << pink() << "\n  Total  " << reset_color() << students.size()
         << " students (" << without_project << " without project)\n";
}

void CalificationProgram::calificate_criterion(Student &student,
                                               int criterion_id)
{
    Criterion *criterion = student.get_calification().find_criterion(criterion_id);
    if (criterion == nullptr)
    {
        cout << "Error: Criterion " << Criterion::format_id(criterion_id)
             << " does not exist.\n";
        return;
    }
    cout << pink() << "  Criterion  " << reset_color()
         << criterion->get_id() << "\n";
    cout << "  " << criterion->get_description() << "\n";

    while (cin)
    {
        cout << bold_pink() << "  Achieved [" << fixed << setprecision(2)
             << criterion->get_base_score() << "] \u203a " << reset_color()
             << flush;
        string input;
        if (!getline(cin, input))
        {
            return;
        }
        try
        {
            criterion->set_achieved_score(parse_input_number(input));
            student.save_raw_note();
            return;
        }
        catch (const invalid_argument &error)
        {
            cout << "Error: " << error.what() << ". Try again.\n";
        }
    }
}

void CalificationProgram::calificate_deduction(Student &student)
{
    const vector<Deduction> &deductions =
            student.get_calification().get_deductions();
    print_section_title("DEDUCTIONS");
    if (deductions.empty())
    {
        cout << "  No deductions are available.\n";
        return;
    }

    for (size_t index = 0; index < deductions.size(); ++index)
    {
        const Deduction &deduction = deductions[index];
        const char *row_color = list_item_color(index);
        cout << row_color << "  " << deduction.get_id() << ". ["
             << base_score_color() << fixed << setprecision(2)
             << deduction.get_base_deduct_score() << row_color << "] "
             << deduction.get_description() << reset_color() << "\n";
    }

    cout << bold_pink() << "\n  Deduction ID \u203a " << reset_color()
         << flush;
    string id;
    if (!getline(cin, id))
    {
        return;
    }

    Deduction *deduction = student.get_calification().find_deduction(trim(id));
    if (deduction == nullptr)
    {
        cout << "Error: Deduction does not exist.\n";
        return;
    }
    while (cin)
    {
        cout << bold_pink() << "  Applied [" << fixed << setprecision(2)
             << deduction->get_base_deduct_score() << "] \u203a "
             << reset_color() << flush;
        string input;
        if (!getline(cin, input))
        {
            return;
        }
        try
        {
            deduction->set_achieved_deduct_score(parse_input_number(input));
            student.save_raw_note();
            return;
        }
        catch (const invalid_argument &error)
        {
            cout << "Error: " << error.what() << ". Try again.\n";
        }
    }
}

void CalificationProgram::register_observation(Student &student)
{
    vector<Observation> general_observations = load_general_observations();
    Rubric &student_rubric = student.get_calification();
    student_rubric.refresh_observations(general_observations);
    student.save_raw_note();

    print_section_title("OBSERVATION LIBRARY");
    for (size_t index = 0; index < general_observations.size(); ++index)
    {
        const Observation &observation = general_observations[index];
        cout << list_item_color(index) << "  " << observation.get_id()
             << ". " << observation.get_description() << reset_color()
             << "\n";
    }
    cout << bold_pink() << "\n  Observation (On, text, or n for new) \u203a "
         << reset_color() << flush;

    string input;
    if (!getline(cin, input))
    {
        return;
    }
    const string option = trim(input);
    if (option == "n")
    {
        cout << bold_pink() << "  New reusable observation \u203a "
             << reset_color() << flush;
        string description;
        if (!getline(cin, description))
        {
            return;
        }
        description = trim(description);
        if (description.empty())
        {
            cout << "Error: Observation cannot be empty.\n";
            return;
        }
        student_rubric.add_observation(
                create_general_observation(description));
    }
    else
    {
        const string observation_id = normalize_observation_id(option);
        const auto found = find_observation(general_observations, observation_id);
        if (found != general_observations.end())
        {
            student_rubric.add_observation(*found);
        }
        else if (is_observation_id(observation_id))
        {
            cout << "Error: Observation " << observation_id
                 << " does not exist.\n";
            return;
        }
        else if (option.empty())
        {
            cout << "Error: Observation cannot be empty.\n";
            return;
        }
        else
        {
            student_rubric.add_observation(Observation("", option));
        }
    }
    student.save_raw_note();
}

void CalificationProgram::delete_observation(Student &student)
{
    Rubric &student_rubric = student.get_calification();
    const vector<Observation> &observations = student_rubric.get_observations();

    print_section_title("DELETE OBSERVATION");
    if (observations.empty())
    {
        cout << "  This student has no registered observations.\n";
        return;
    }

    for (size_t index = 0; index < observations.size(); ++index)
    {
        const Observation &observation = observations[index];
        cout << list_item_color(index) << "  [" << index + 1 << "] ";
        if (!observation.get_id().empty())
        {
            cout << observation.get_id() << ". ";
        }
        cout << observation.get_description() << reset_color() << "\n";
    }

    while (cin)
    {
        cout << bold_pink()
             << "\n  Observation number (q to cancel) \u203a "
             << reset_color() << flush;
        string input;
        if (!getline(cin, input))
        {
            return;
        }
        input = trim(input);
        if (input == "q" or input == "cancel")
        {
            cout << "  Deletion cancelled.\n";
            return;
        }

        size_t read = 0;
        unsigned long selection = 0;
        try
        {
            selection = stoul(input, &read);
        }
        catch (const exception &)
        {
            read = 0;
        }
        if (input.empty() or read != input.size() or selection == 0 or
            selection > observations.size())
        {
            cout << "  Enter a number from 1 to " << observations.size()
                 << ", or q to cancel.\n";
            continue;
        }

        const size_t index = static_cast<size_t>(selection - 1);
        const string removed_description = observations[index].get_description();
        if (!student_rubric.remove_observation(index))
        {
            throw runtime_error("Could not remove the selected observation");
        }
        student.save_raw_note();
        cout << achieved_score_color() << "  \u2713 Removed  " << reset_color()
             << removed_description << "\n";
        return;
    }
}

void CalificationProgram::calificate_students()
{
    ensure_students_loaded();
    vector<size_t> grading_students;
    for (size_t index = 0; index < students.size(); ++index)
    {
        if (students[index].has_project())
        {
            grading_students.push_back(index);
        }
    }

    if (grading_students.empty())
    {
        cout << "No students with a project to grade.\n";
        return;
    }

    const auto first_blank = find_if(
            grading_students.begin(), grading_students.end(),
            [this](size_t index)
    {
        return students[index].get_state() == StudentState::BLANK;
    });
    const auto first_incomplete = find_if(
            grading_students.begin(), grading_students.end(),
            [this](size_t index)
    {
        return students[index].get_state() == StudentState::INCOMPLETE;
    });

    size_t current = 0;
    if (first_blank != grading_students.end())
    {
        current = static_cast<size_t>(
                distance(grading_students.begin(), first_blank));
    }
    else if (first_incomplete != grading_students.end())
    {
        current = static_cast<size_t>(
                distance(grading_students.begin(), first_incomplete));
    }

    while (cin)
    {
        Student &student = students[grading_students[current]];

        print_student_card(student, "CURRENT STUDENT");
        cout << soft_pink()
             << "\n  Commands  criterion #  d  o  do  sc [width]  sd  so  <  >  cod  help  q\n"
             << reset_color();

        while (cin)
        {
            cout << bold_pink() << "\n  grade \u203a " << reset_color() << flush;
            string option;
            if (!getline(cin, option))
            {
                return;
            }
            option = trim(option);
            const string command = command_name(option);
            if (option == "d")
            {
                calificate_deduction(student);
            }
            else if (option == "o")
            {
                register_observation(student);
            }
            else if (option == "do")
            {
                delete_observation(student);
            }
            else if (command == "sc")
            {
                try
                {
                    const size_t display_width = criteria_display_width(
                            option, report_width);
                    print_student_card(student, "CRITERIA FOR STUDENT");
                    cout << "\n" << pink();
                    student.get_calification().print_current_criteria(
                            cout, display_width, achieved_score_color(),
                            base_score_color(), pink(), list_line_colors());
                    cout << reset_color();
                }
                catch (const invalid_argument &error)
                {
                    cout << "Error: " << error.what() << ".\n";
                }
            }
            else if (option == "sd")
            {
                cout << pink();
                student.get_calification().print_current_deductions(
                        cout, report_width, list_line_colors(), pink());
                cout << reset_color();
            }
            else if (option == "so")
            {
                cout << pink();
                student.get_calification().print_current_observations(
                        cout, report_width, list_line_colors(), pink());
                cout << reset_color();
            }
            else if (command == "sd" or command == "so")
            {
                cout << "Error: Usage: " << command << ".\n";
            }
            else if (option == "show")
            {
                cout << "The show command is now split into sc, sd, and so.\n";
            }
            else if (option == "<")
            {
                current = (current + 1) % grading_students.size();
                break;
            }
            else if (option == ">")
            {
                current = (current + grading_students.size() - 1) %
                          grading_students.size();
                break;
            }
            else if (option == "cod")
            {
                cout << bold_pink() << "  Student code \u203a "
                     << reset_color() << flush;
                string code;
                if (!getline(cin, code))
                {
                    return;
                }
                code = trim(code);
                const auto found = find_if(
                        grading_students.begin(), grading_students.end(),
                        [&code, this](size_t index)
                {
                    return students[index].get_code() == code;
                });
                if (found == grading_students.end())
                {
                    cout << "Error: No project found for student code "
                         << code << ".\n";
                }
                else
                {
                    current = static_cast<size_t>(
                            distance(grading_students.begin(), found));
                    break;
                }
            }
            else if (option == "q")
            {
                return;
            }
            else if (option == "help")
            {
                print_section_title("GRADING COMMANDS");
                cout << list_item_color(0)
                     << "  number      Grade a criterion (1 selects 01)"
                     << reset_color() << "\n";
                cout << list_item_color(1) << "  d           Apply a deduction"
                     << reset_color() << "\n";
                cout << list_item_color(2)
                     << "  o           Register an observation"
                     << reset_color() << "\n";
                cout << list_item_color(3)
                     << "  do          Delete a registered observation"
                     << reset_color() << "\n";
                cout << list_item_color(4)
                     << "  sc [width]  Show criteria (default width: "
                     << report_width << ")" << reset_color() << "\n";
                cout << list_item_color(5) << "  sd          Show deductions"
                     << reset_color() << "\n";
                cout << list_item_color(6) << "  so          Show observations"
                     << reset_color() << "\n";
                cout << list_item_color(7)
                     << "  < / >       Next / previous student"
                     << reset_color() << "\n";
                cout << list_item_color(8)
                     << "  cod         Go to a student code"
                     << reset_color() << "\n";
                cout << list_item_color(9)
                     << "  q           Return to the main menu"
                     << reset_color() << "\n";
            }
            else
            {
                try
                {
                    const int criterion_id = Criterion::parse_id(option);
                    calificate_criterion(student, criterion_id);
                }
                catch (const invalid_argument &error)
                {
                    cout << "Error: " << error.what()
                         << ". Enter a criterion number or a listed option.\n";
                }
            }
        }
    }
}

void CalificationProgram::write_feedback()
{
    ensure_students_loaded();
    const vector<Observation> general_observations =
            load_general_observations();
    size_t written = 0;

    print_section_title("WRITING FEEDBACK");
    for (auto &student: students)
    {
        if (!student.has_project() or student.get_state() != StudentState::READY)
        {
            continue;
        }
        student.get_calification().refresh_observations(general_observations);
        student.save_raw_note();
        const path output = student.write_feedback(report_width);
        cout << list_item_color(written) << "  \u2713 " << output.string()
             << reset_color() << "\n";
        ++written;
    }
    cout << pink() << "\n  Total  " << reset_color()
         << written << " feedback files\n";
}

void CalificationProgram::run()
{
    print_welcome();
    while (cin)
    {
        print_menu();
        string option;
        if (!getline(cin, option))
        {
            break;
        }
        option = trim(option);

        try
        {
            if (option == "a")
            {
                show_students();
            }
            else if (option == "b")
            {
                calificate_students();
            }
            else if (option == "c")
            {
                write_feedback();
            }
            else if (option == "d")
            {
                cout << pink() << "  Send emails  " << reset_color()
                     << "Not implemented yet.\n";
            }
            else if (option == "q")
            {
                break;
            }
            else
            {
                cout << pink() << "  Unknown option.  " << reset_color()
                     << "Choose a, b, c, d, or q.\n";
            }
        }
        catch (const exception &error)
        {
            cerr << "Error: " << error.what() << "\n";
        }
    }
}

path resolve_lab_folder(const string &evaluation_name)
{
    const path requested(evaluation_name);
    error_code error;
    if (requested.is_absolute() and is_directory(requested, error))
    {
        return weakly_canonical(requested);
    }
    if (is_directory(requested, error) and
        is_directory(requested / "meta", error))
    {
        return weakly_canonical(requested);
    }

    path current = current_path();
    while (true)
    {
        const path candidate = current / "data" / requested;
        if (is_directory(candidate, error))
        {
            return weakly_canonical(candidate);
        }
        if (current == current.root_path())
        {
            break;
        }
        current = current.parent_path();
    }
    return current_path() / "data" / requested;
}
