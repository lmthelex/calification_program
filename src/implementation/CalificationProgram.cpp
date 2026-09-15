#include "../headers/CalificationProgram.hpp"

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
                    const auto found = find_if(
                            rubric.get_criteria().begin(),
                            rubric.get_criteria().end(),
                            [&id](const Criterion &criterion)
                    {
                        return criterion.get_id() == id;
                    });
                    if (found != rubric.get_criteria().end())
                    {
                        base_score = found->get_base_score();
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
    cout << "Welcome to Calification program.\n";
    cout << "Evaluating \"" << evaluation_name << "\".\n";
    cout << "___\n";
    cout << "Rubric: Loaded\n";
    rubric.print(cout, report_width);
    cout << "___\n";
}

void CalificationProgram::print_menu() const
{
    cout << "Menu:\n";
    cout << "a. Show students\n";
    cout << "b. Calificate students\n";
    cout << "c. Write feedback\n";
    cout << "d. Send emails\n";
    cout << "Option: " << flush;
}

void CalificationProgram::show_students()
{
    load_students();

    cout << "___\n";
    cout << "List of students:\n";
    cout << left << setw(12) << "Code"
         << setw(24) << "Last_name"
         << setw(24) << "First_name"
         << setw(10) << "Project"
         << "State\n";

    size_t without_project = 0;
    for (const auto &student: students)
    {
        if (!student.has_project())
        {
            ++without_project;
        }
        cout << left << setw(12) << student.get_code()
             << setw(24) << student.get_last_name()
             << setw(24) << student.get_first_name()
             << setw(10) << (student.has_project() ? "true" : "false")
             << student.get_state_name() << "\n";
    }
    cout << "\nTotal: " << students.size() << " (" << without_project
         << " without project)\n";
    cout << "___\n";
}

void CalificationProgram::calificate_criterion(Student &student)
{
    cout << "Criterion choosen: " << flush;
    string id;
    if (!getline(cin, id))
    {
        return;
    }

    Criterion *criterion = student.get_calification().find_criterion(trim(id));
    if (criterion == nullptr)
    {
        cout << "Error: Criterion does not exist.\n";
        return;
    }
    cout << "\t" << criterion->get_description() << "\n";

    while (cin)
    {
        cout << "Achieved [" << fixed << setprecision(2)
             << criterion->get_base_score() << "]: " << flush;
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
    cout << "Deduction choosen: " << flush;
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
    cout << "\t" << deduction->get_description() << "\n";

    while (cin)
    {
        cout << "Achieved [" << fixed << setprecision(2)
             << deduction->get_base_deduct_score() << "]: " << flush;
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

    cout << "___\n";
    cout << "Observations already registered:\n";
    for (const auto &observation: general_observations)
    {
        cout << " " << observation.get_id() << ". "
             << observation.get_description() << "\n";
    }
    cout << "Observation type (On, string, n): " << flush;

    string input;
    if (!getline(cin, input))
    {
        return;
    }
    const string option = trim(input);
    if (option == "n")
    {
        cout << "You are registering a new general observation: " << flush;
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

void CalificationProgram::calificate_students()
{
    ensure_students_loaded();
    vector<size_t> pending_students;
    for (size_t index = 0; index < students.size(); ++index)
    {
        if (students[index].has_project() and
            students[index].get_state() != StudentState::READY)
        {
            pending_students.push_back(index);
        }
    }

    if (pending_students.empty())
    {
        cout << "No students with a pending project calification.\n";
        cout << "___\n";
        return;
    }

    size_t current = 0;
    while (cin)
    {
        Student &student = students[pending_students[current]];

        cout << "___\n";
        cout << "Student: " << student.get_last_name() << " "
             << student.get_first_name() << "\n";
        cout << "Code: " << student.get_code() << "\n";
        cout << "Path: " << student.get_project_folder()->string() << "\n";

        while (cin)
        {
            cout << "Option (c,d,o)(<,>,cod)(help): " << flush;
            string option;
            if (!getline(cin, option))
            {
                return;
            }
            option = trim(option);
            if (option == "c")
            {
                calificate_criterion(student);
            }
            else if (option == "d")
            {
                calificate_deduction(student);
            }
            else if (option == "o")
            {
                register_observation(student);
            }
            else if (option == "<")
            {
                current = (current + 1) % pending_students.size();
                break;
            }
            else if (option == ">")
            {
                current = (current + pending_students.size() - 1) %
                          pending_students.size();
                break;
            }
            else if (option == "cod")
            {
                cout << "Student code: " << flush;
                string code;
                if (!getline(cin, code))
                {
                    return;
                }
                code = trim(code);
                const auto found = find_if(
                        pending_students.begin(), pending_students.end(),
                        [&code, this](size_t index)
                {
                    return students[index].get_code() == code;
                });
                if (found == pending_students.end())
                {
                    cout << "Error: No pending project for student code "
                         << code << ".\n";
                }
                else
                {
                    current = static_cast<size_t>(
                            distance(pending_students.begin(), found));
                    break;
                }
            }
            else if (option == "q")
            {
                cout << "___\n";
                return;
            }
            else if (option == "help")
            {
                cout << "c - Criteria calification\n";
                cout << "d - Discount applied\n";
                cout << "o - Observation registration\n";
                cout << "< - Next student (forward)\n";
                cout << "> - Previous student (backward)\n";
                cout << "cod - Go directly to a student code\n";
                cout << "q - Return to the main menu\n";
            }
            else
            {
                const auto found = find_if(
                        pending_students.begin(), pending_students.end(),
                        [&option, this](size_t index)
                {
                    return students[index].get_code() == option;
                });
                if (found == pending_students.end())
                {
                    cout << "Error: Unknown calification option or student code.\n";
                }
                else
                {
                    current = static_cast<size_t>(
                            distance(pending_students.begin(), found));
                    break;
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

    cout << "___\n";
    for (auto &student: students)
    {
        if (!student.has_project() or student.get_state() != StudentState::READY)
        {
            continue;
        }
        student.get_calification().refresh_observations(general_observations);
        student.save_raw_note();
        const path output = student.write_feedback(report_width);
        cout << "Feedback written: " << output.string() << "\n";
        ++written;
    }
    cout << "Total feedback files: " << written << "\n";
    cout << "___\n";
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
                cout << "Send emails is not implemented yet.\n___\n";
            }
            else if (option == "q")
            {
                break;
            }
            else
            {
                cout << "Error: Unknown menu option.\n___\n";
            }
        }
        catch (const exception &error)
        {
            cerr << "Error: " << error.what() << "\n___\n";
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
