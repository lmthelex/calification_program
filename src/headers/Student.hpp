#ifndef CALIFICATION_PROGRAM_STUDENT_HPP
#define CALIFICATION_PROGRAM_STUDENT_HPP

#include "PdfWriter.hpp"
#include "Rubric.hpp"
#include "utils.hpp"

enum class StudentState
{
    BLANK,
    INCOMPLETE,
    READY
};

class Student
{
private:
    string code;
    string last_name;
    string first_name;
    vector<string> emails;
    path student_folder;
    optional<path> project_folder;
    Rubric calification;

    static string folder_component(const string &value);
    optional<path> find_project_folder() const;
    optional<path> find_archive(const path &raw_folder) const;
    bool extract_archive(const path &archive) const;

public:
    Student(string code_, string last_name_, string first_name_,
            vector<string> emails_, const Rubric &rubric);

    static vector<Student> read_students(istream &students_file,
                                         const Rubric &rubric);

    const string &get_code() const;
    const string &get_last_name() const;
    const string &get_first_name() const;
    const vector<string> &get_emails() const;
    const path &get_student_folder() const;
    const optional<path> &get_project_folder() const;
    Rubric &get_calification();
    const Rubric &get_calification() const;
    bool has_project() const;
    StudentState get_state() const;
    string get_state_name() const;

    void prepare(const path &lab_folder);
    void save_raw_note() const;
    path write_feedback() const;
};

#endif //CALIFICATION_PROGRAM_STUDENT_HPP
