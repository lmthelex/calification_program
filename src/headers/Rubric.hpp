//
// Created by lmthelex on 21/09/2025.
//

#ifndef LAB02_TP_RUBRIC_HPP
#define LAB02_TP_RUBRIC_HPP

#include "Criterion.hpp"
#include "Deduction.hpp"
#include "Observation.hpp"
#include "read_functions.hpp"
#include "utils.hpp"

class Rubric
{
private:
    string student_name;
    vector<Criterion> criteria;
    vector<Deduction> deductions;
    vector<Observation> general_observations;
    vector<Observation> observations;

    //private methods
    void read_scored_block(ifstream &rubric_file);
    void read_unscored_block(ifstream &rubric_file);

    void read_student_name(ifstream &rubric_file);
    void read_evaluated_scored_block(ifstream &rubric_file);
    void read_evaluated_unscored_block(ifstream &rubric_file);

    Criterion *find_criteria(const string &id);
    Deduction *find_deduction(const string &id);
    Observation *find_observation(const string &id);

public:
    //constructor
    Rubric()
        : criteria(), deductions(), general_observations()
    {}
    Rubric(const Rubric &other)
           : criteria(other.criteria),
             deductions(other.deductions),
             general_observations(other.general_observations){}
    //public methods
    void read_rubric(ifstream &rubric_file);
    void read_evaluated_rubric(ifstream &rubric_file);
    void print(ofstream &evaluated_rubric);

};

#endif //LAB02_TP_RUBRIC_HPP
