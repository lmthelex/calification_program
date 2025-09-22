//
// Created by lmthelex on 21/09/2025.
//

#include "..\headers\Deduction.hpp"

//getters and setters
double Deduction::set_achieved_deduct_score(double achieved_)
{
    achieved_deduct_score = achieved_;
}

double Deduction::get_base_deduct_score() const
{
    return base_deduct_score;
}

double Deduction::get_achieved_deduct_score() const
{
    return achieved_deduct_score;
}

//methods
void Deduction::print(ofstream &file) const
{
    file << left << setw(6) << id
            << right << setw(10) << fixed << setprecision(2) << base_deduct_score
            << right << setw(10) << fixed << setprecision(2) << achieved_deduct_score
            << string(5, ' ') << left << description << "\n";
}
