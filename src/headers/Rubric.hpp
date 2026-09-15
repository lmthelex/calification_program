//
// Created by lmthelex on 21/09/2025.
//

#ifndef CALIFICATION_PROGRAM_RUBRIC_HPP
#define CALIFICATION_PROGRAM_RUBRIC_HPP

#include "Criterion.hpp"
#include "Deduction.hpp"
#include "Observation.hpp"
#include "utils.hpp"

class Rubric
{
private:
    string student_name;
    vector<Criterion> criteria;
    vector<Deduction> deductions;
    vector<Observation> observations;

public:
    Rubric() = default;
    Rubric(const Rubric &) = default;
    Rubric &operator=(const Rubric &) = default;

    //getters and setters
    const string &get_student_name() const;
    void set_student_name(string student_name_);
    double get_base_score() const;
    double get_achieved_score() const;
    const vector<Criterion> &get_criteria() const;
    const vector<Deduction> &get_deductions() const;
    const vector<Observation> &get_observations() const;
    Criterion *find_criterion(int numeric_id);
    Criterion *find_criterion(const string &id);
    Deduction *find_deduction(const string &id);

    //state
    bool has_any_calification() const;
    bool is_ready() const;

    //input and output
    void read_rubric(istream &rubric_file);
    void read_raw_note(istream &raw_note_file);
    void write_raw_note(ostream &raw_note_file) const;
    void print(ostream &output, size_t report_width,
               const vector<string> &line_colors = {},
               const string &default_color = "") const;
    void print_current_criteria(
            ostream &output, size_t display_width,
            const string &achieved_color = "",
            const string &base_color = "",
            const string &default_color = "",
            const vector<string> &line_colors = {}) const;
    void print_current_deductions(
            ostream &output, size_t report_width,
            const vector<string> &line_colors = {},
            const string &default_color = "") const;
    void print_current_observations(
            ostream &output, size_t report_width,
            const vector<string> &line_colors = {},
            const string &default_color = "") const;
    void print_feedback(ostream &output, size_t report_width) const;

    //observations selected for one student
    void add_observation(const Observation &observation);
    bool remove_observation(size_t index);
    void refresh_observations(const vector<Observation> &general_observations);
};

#endif //CALIFICATION_PROGRAM_RUBRIC_HPP
