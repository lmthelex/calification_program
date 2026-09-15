#ifndef CALIFICATION_PROGRAM_HPP
#define CALIFICATION_PROGRAM_HPP

#include "Student.hpp"
#include "utils.hpp"

class CalificationProgram
{
private:
    string evaluation_name;
    path lab_folder;
    Rubric rubric;
    vector<Student> students;
    bool students_loaded;
    size_t report_width;

    void load_rubric();
    void load_students();
    void ensure_students_loaded();
    vector<Observation> load_general_observations() const;
    Observation create_general_observation(const string &description) const;

    void print_welcome() const;
    void print_menu() const;
    void show_students();
    void calificate_students();
    void calificate_criterion(Student &student);
    void calificate_deduction(Student &student);
    void register_observation(Student &student);
    void write_feedback();

public:
    CalificationProgram(string evaluation_name_, path lab_folder_,
                        size_t report_width_);
    void run();
};

path resolve_lab_folder(const string &evaluation_name);

#endif //CALIFICATION_PROGRAM_HPP
