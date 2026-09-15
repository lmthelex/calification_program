//
// Created by lmthelex on 21/09/2025.
//

#include "../headers/Deduction.hpp"

//getters and setters
void Deduction::set_achieved_deduct_score(double achieved_)
{
    if (!isfinite(achieved_) or achieved_ > 0.0 or achieved_ < base_deduct_score)
    {
        throw invalid_argument("Deduction must be between its base value and 0");
    }
    achieved_deduct_score = achieved_;
}

void Deduction::clear_achieved_deduct_score()
{
    achieved_deduct_score.reset();
}

double Deduction::get_base_deduct_score() const
{
    return base_deduct_score;
}

double Deduction::get_achieved_deduct_score() const
{
    return achieved_deduct_score.value_or(0.0);
}

bool Deduction::has_achieved_deduct_score() const
{
    return achieved_deduct_score.has_value();
}

//methods
void Deduction::print(ostream &file) const
{
    file << left << setw(6) << id
            << right << setw(10) << fixed << setprecision(2) << base_deduct_score
            << right << setw(10) << fixed << setprecision(2)
            << achieved_deduct_score.value_or(0.0)
            << string(5, ' ') << left << description << "\n";
}
