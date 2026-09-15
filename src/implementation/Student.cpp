#include "../headers/Student.hpp"

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

vector<string> split(const string &value, char separator)
{
    vector<string> fields;
    string field;
    istringstream input(value);
    while (getline(input, field, separator))
    {
        fields.push_back(trim(field));
    }
    if (!value.empty() and value.back() == separator)
    {
        fields.emplace_back();
    }
    return fields;
}

string lower_copy(string value)
{
    transform(value.begin(), value.end(), value.begin(), [](unsigned char c)
    {
        return static_cast<char>(tolower(c));
    });
    return value;
}

string shell_quote(const string &value)
{
    string quoted = "'";
    for (char character: value)
    {
        if (character == '\'')
        {
            quoted += "'\\''";
        }
        else
        {
            quoted += character;
        }
    }
    return quoted + "'";
}

size_t count_main_files(const path &folder)
{
    size_t count = 0;
    error_code error;
    recursive_directory_iterator iterator(
            folder, directory_options::skip_permission_denied, error);
    const recursive_directory_iterator end;

    while (!error and iterator != end)
    {
        if (iterator->is_regular_file(error) and
            iterator->path().filename() == "main.cpp")
        {
            ++count;
            if (count >= 2)
            {
                return count;
            }
        }
        iterator.increment(error);
    }
    return count;
}
}

Student::Student(string code_, string last_name_, string first_name_,
                 vector<string> emails_, const Rubric &rubric)
    : code(std::move(code_))
    , last_name(std::move(last_name_))
    , first_name(std::move(first_name_))
    , emails(std::move(emails_))
    , calification(rubric)
{
    calification.set_student_name(last_name + " " + first_name);
}

vector<Student> Student::read_students(istream &students_file,
                                       const Rubric &rubric)
{
    vector<Student> result;
    unordered_set<string> codes;
    string line;
    size_t line_number = 0;

    while (getline(students_file, line))
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

        const vector<string> fields = split(line, ';');
        string student_code;
        string student_last_name;
        string student_first_name;
        string email_field;

        if (fields.size() == 3)
        {
            student_code = fields[0];
            const auto comma = fields[1].find(',');
            if (comma == string::npos)
            {
                throw runtime_error("Expected 'LAST NAME, FIRST NAME' on students.txt line " +
                                    to_string(line_number));
            }
            student_last_name = trim(fields[1].substr(0, comma));
            student_first_name = trim(fields[1].substr(comma + 1));
            email_field = fields[2];
        }
        else if (fields.size() == 4)
        {
            student_code = fields[0];
            student_last_name = fields[1];
            student_first_name = fields[2];
            email_field = fields[3];
        }
        else
        {
            throw runtime_error("Malformed students.txt line " +
                                to_string(line_number));
        }

        if (student_code.empty() or student_last_name.empty() or
            student_first_name.empty())
        {
            throw runtime_error("Missing student data on students.txt line " +
                                to_string(line_number));
        }
        if (student_code == "." or student_code == ".." or
            student_code.find('/') != string::npos or
            student_code.find('\\') != string::npos)
        {
            throw runtime_error("Invalid student code on students.txt line " +
                                to_string(line_number));
        }
        if (!codes.insert(student_code).second)
        {
            throw runtime_error("Duplicate student code: " + student_code);
        }

        vector<string> student_emails = split(email_field, ',');
        student_emails.erase(remove_if(student_emails.begin(), student_emails.end(),
                                       [](const string &email)
        {
            return email.empty();
        }), student_emails.end());
        result.emplace_back(student_code, student_last_name, student_first_name,
                            student_emails, rubric);
    }

    if (!students_file.eof())
    {
        throw runtime_error("Could not read students.txt");
    }
    return result;
}

const string &Student::get_code() const
{
    return code;
}

const string &Student::get_last_name() const
{
    return last_name;
}

const string &Student::get_first_name() const
{
    return first_name;
}

const vector<string> &Student::get_emails() const
{
    return emails;
}

const path &Student::get_student_folder() const
{
    return student_folder;
}

const optional<path> &Student::get_project_folder() const
{
    return project_folder;
}

Rubric &Student::get_calification()
{
    return calification;
}

const Rubric &Student::get_calification() const
{
    return calification;
}

bool Student::has_project() const
{
    return project_folder.has_value();
}

StudentState Student::get_state() const
{
    if (calification.is_ready())
    {
        return StudentState::READY;
    }
    if (calification.has_any_calification())
    {
        return StudentState::INCOMPLETE;
    }
    return StudentState::BLANK;
}

string Student::get_state_name() const
{
    switch (get_state())
    {
        case StudentState::READY:
            return "READY";
        case StudentState::INCOMPLETE:
            return "INCOMPLETE";
        case StudentState::BLANK:
            return "BLANK";
    }
    return "BLANK";
}

string Student::folder_component(const string &value)
{
    string result;
    bool previous_was_separator = false;
    for (unsigned char character: value)
    {
        const bool separator = isspace(character) or character == '/' or
                               character == '\\';
        if (separator)
        {
            if (!result.empty() and !previous_was_separator)
            {
                result += '_';
            }
            previous_was_separator = true;
        }
        else if (character >= 32)
        {
            result += static_cast<char>(character);
            previous_was_separator = false;
        }
    }
    while (!result.empty() and result.back() == '_')
    {
        result.pop_back();
    }
    if (result.empty())
    {
        throw runtime_error("A student name cannot produce an empty folder name");
    }
    return result;
}

optional<path> Student::find_project_folder() const
{
    error_code error;
    directory_iterator iterator(student_folder, error);
    const directory_iterator end;

    while (!error and iterator != end)
    {
        if (iterator->is_directory(error) and count_main_files(iterator->path()) >= 2)
        {
            return iterator->path();
        }
        iterator.increment(error);
    }
    if (count_main_files(student_folder) >= 2)
    {
        return student_folder;
    }
    return nullopt;
}

optional<path> Student::find_archive(const path &raw_folder) const
{
    vector<path> matches;
    error_code error;
    directory_iterator iterator(raw_folder, error);
    const directory_iterator end;

    while (!error and iterator != end)
    {
        if (iterator->is_regular_file(error) and
            lower_copy(iterator->path().extension().string()) == ".zip" and
            iterator->path().filename().string().find(code) != string::npos)
        {
            matches.push_back(iterator->path());
        }
        iterator.increment(error);
    }
    if (matches.empty())
    {
        return nullopt;
    }
    sort(matches.begin(), matches.end());
    return matches.front();
}

bool Student::extract_archive(const path &archive) const
{
    const string command = "unzip -qq -o " + shell_quote(archive.string()) +
                           " -d " + shell_quote(student_folder.string());
    return system(command.c_str()) == 0;
}

void Student::prepare(const path &lab_folder)
{
    const path projects_folder = lab_folder / "projects";
    create_directories(projects_folder);
    student_folder = projects_folder /
            (code + "_" + folder_component(last_name) + "_" +
             folder_component(first_name));
    create_directories(student_folder);

    project_folder = find_project_folder();
    if (!project_folder)
    {
        const optional<path> archive = find_archive(lab_folder / "raw");
        if (archive and extract_archive(*archive))
        {
            project_folder = find_project_folder();
        }
    }

    if (!project_folder)
    {
        return;
    }

    const path raw_note = student_folder / "raw_note.txt";
    error_code error;
    const bool needs_initialization = !exists(raw_note) or
                                      file_size(raw_note, error) == 0;
    if (needs_initialization)
    {
        save_raw_note();
        return;
    }

    ifstream input(raw_note);
    if (!input.is_open())
    {
        throw runtime_error("Could not open " + raw_note.string());
    }
    calification.read_raw_note(input);
}

void Student::save_raw_note() const
{
    if (student_folder.empty())
    {
        throw runtime_error("Cannot save a student before preparing its folder");
    }

    const path destination = student_folder / "raw_note.txt";
    const path temporary = student_folder / "raw_note.txt.tmp";
    ofstream output(temporary, ios::trunc);
    if (!output.is_open())
    {
        throw runtime_error("Could not write " + temporary.string());
    }
    calification.write_raw_note(output);
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
}

path Student::write_feedback() const
{
    if (!has_project() or get_state() != StudentState::READY)
    {
        throw runtime_error("Feedback can only be written for a READY project");
    }

    const path destination = student_folder / "final_note.pdf";
    const path temporary = student_folder / "final_note.pdf.tmp";
    ofstream output(temporary, ios::binary | ios::trunc);
    if (!output.is_open())
    {
        throw runtime_error("Could not write " + temporary.string());
    }
    ostringstream feedback;
    calification.print_feedback(feedback);
    write_pdf(output, feedback.str());
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
    return destination;
}
