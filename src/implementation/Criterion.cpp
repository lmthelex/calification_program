//
// Created by lmthelex on 21/09/2025.
//

#include "../headers/Criterion.hpp"

int Criterion::parse_id(const string &id)
{
    if (id.empty() or !all_of(id.begin(), id.end(), [](unsigned char character)
    {
        return isdigit(character);
    }))
    {
        throw invalid_argument("Criterion id must contain only digits");
    }

    size_t read = 0;
    unsigned long numeric_id = 0;
    try
    {
        numeric_id = stoul(id, &read);
    }
    catch (const exception &)
    {
        throw invalid_argument("Criterion id is outside the supported range");
    }
    if (read != id.size() or numeric_id == 0 or
        numeric_id > static_cast<unsigned long>(numeric_limits<int>::max()))
    {
        throw invalid_argument("Criterion id must be a positive integer");
    }
    return static_cast<int>(numeric_id);
}

string Criterion::format_id(int numeric_id)
{
    if (numeric_id <= 0)
    {
        throw invalid_argument("Criterion id must be a positive integer");
    }
    ostringstream formatted;
    formatted << setw(2) << setfill('0') << numeric_id;
    return formatted.str();
}

int Criterion::get_numeric_id() const
{
    return parse_id(id);
}

//getters and setters
void Criterion::set_achieved_score(double achieved_)
{
    if (!isfinite(achieved_) or achieved_ < 0.0 or achieved_ > base_score)
    {
        throw invalid_argument("Criterion score must be between 0 and its base score");
    }
    achieved_score = achieved_;
}

void Criterion::restore_achieved_score(double achieved_)
{
    if (!isfinite(achieved_))
    {
        throw invalid_argument("Stored criterion score must be finite");
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
