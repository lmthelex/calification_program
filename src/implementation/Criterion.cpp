//
// Created by lmthelex on 21/09/2025.
//

#include "../headers/Criterion.hpp"

//getters and setters
void Criterion::set_achieved_score(double achieved_)
{
    if (!isfinite(achieved_) or achieved_ < 0.0 or achieved_ > base_score)
    {
        throw invalid_argument("Criterion score must be between 0 and its base score");
    }
    achieved_score = achieved_;
}

void Criterion::clear_achieved_score()
{
    achieved_score.reset();
}

double Criterion::get_base_score() const
{
    return base_score;
}

double Criterion::get_achieved_score() const
{
    return achieved_score.value_or(0.0);
}

bool Criterion::has_achieved_score() const
{
    return achieved_score.has_value();
}

//methods
void Criterion::print(ostream &file) const
{
    file << left << setw(6) << id
            << right << setw(10) << fixed << setprecision(2) << base_score
            << right << setw(10) << fixed << setprecision(2)
            << achieved_score.value_or(0.0)
            << string(5, ' ') << left << description << "\n";
}
