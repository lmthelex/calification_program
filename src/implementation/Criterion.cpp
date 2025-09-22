//
// Created by lmthelex on 21/09/2025.
//

#include "..\headers\Criterion.hpp"

//getters and setters
void Criterion::set_achieved_score(double achieved_)
{
    achieved_score = achieved_;
}

double Criterion::get_base_score() const
{
    return base_score;
}

double Criterion::get_achieved_score() const
{
    return achieved_score;
}

//methods
void Criterion::print(ofstream &file) const
{
    file << left << setw(6) << id
            << right << setw(10) << fixed << setprecision(2) << base_score
            << right << setw(10) << fixed << setprecision(2) << achieved_score
            << string(5, ' ') << left << description << "\n";
}
