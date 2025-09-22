//
// Created by lmthelex on 21/09/2025.
//

#include "..\headers\Rubric.hpp"

//methods
void Rubric::read_scored_block(ifstream &rubric_file)
{
    string section;

    rubric_file >> section;

    while (true)
    {
        string id;

        rubric_file >> id;

        if (id == "-")
        {
            break;
        }

        char character;
        double score;
        string description;

        rubric_file >> character;
        rubric_file >> score;
        rubric_file >> character;
        getline(rubric_file, description);

        if (section == "Puntaje")
        {
            Criterion criterion(id, description, score);
            criteria.push_back(criterion);
        }
        else if (section == "Descuentos")
        {
            Deduction deduction(id, description, score);
            deductions.push_back(deduction);
        }
    }
}

void Rubric::read_unscored_block(ifstream &rubric_file)
{
    string section;

    rubric_file >> section;

    while (true)
    {
        string id;

        rubric_file >> id;

        if (rubric_file.eof())
        {
            break;
        }

        char character;
        double score;
        string description;

        getline(rubric_file, description);

        Observation observation(id, description);
        general_observations.push_back(observation);
    }
}


void Rubric::read_student_name(ifstream &rubric_file)
{
    string section, name;

    rubric_file >> section;
    getline(rubric_file, student_name);
}

void Rubric::read_evaluated_scored_block(ifstream &rubric_file)
{
    char character;
    string section;
    rubric_file >> character;
    rubric_file >> section;

    while (true)
    {
        string id;

        rubric_file >> id;

        if (id == "-")
        {
            break;
        }

        double base_score, achieved_score;

        rubric_file >> character;
        rubric_file >> base_score;
        rubric_file >> character;
        rubric_file >> achieved_score;

        if (section == "Puntaje")
        {
            if (achieved_score > base_score)
            {
                cerr << "Error: Bad calification in " << student_name << endl;
                cerr << "\t in crteria with id: " << id << endl;
                exit(0);
            }

            if (Criterion *founded_criteria = find_criteria(id))
            {
                founded_criteria->set_achieved_score(achieved_score);
            }
            else
            {
                cerr << "Error: Bad criteria " << id << endl;
                exit(0);
            }
        }
        else if (section == "Descuentos")
        {
            if (achieved_score == base_score or achieved_score  == 0.0)
            {
                cerr << "Error: Bad deduction in " << student_name << endl;
                exit(0);
            }
            if (Deduction *founded_deduction = find_deduction(id))
            {
                founded_deduction->set_achieved_deduct_score(achieved_score);
            }
            else
            {
                cerr << "Error: Bad criteria " << id << endl;
                exit(0);
            }
        }
    }
}

void Rubric::read_evaluated_unscored_block(ifstream &rubric_file)
{
    string section;

    rubric_file >> section >> ws;

    while (true)
    {
        string evaluated_observation;

        getline(rubric_file, evaluated_observation);

        if (rubric_file.eof())
        {
            break;
        }

        if (evaluated_observation[0] == 'O')
        {
            Observation *observation = find_observation(evaluated_observation + ".");

            observations.push_back(*observation);
        }else
        {
            Observation observation("-", evaluated_observation);

            observations.push_back(observation);
        }
    }
}

Criterion *Rubric::find_criteria(const string &id)
{
    for (auto &c: criteria)
    {
        if (c.get_id() == id)
        {
            return &c;
        }
    }
    return nullptr;
}

Deduction *Rubric::find_deduction(const string &id)
{
    for (auto &c: deductions)
    {
        if (c.get_id() == id)
        {
            return &c;
        }
    }
    return nullptr;
}

Observation * Rubric::find_observation(const string &id)
{
    for (auto &c: general_observations)
    {
        if (c.get_id() == id)
        {
            return &c;
        }
    }
    return nullptr;
}


//public methods
void Rubric::read_rubric(ifstream &rubric_file)
{
    read_scored_block(rubric_file);
    read_scored_block(rubric_file);
    read_unscored_block(rubric_file);
}

void Rubric::read_evaluated_rubric(ifstream &rubric_file)
{
    read_student_name(rubric_file);
    read_evaluated_scored_block(rubric_file);
    read_evaluated_scored_block(rubric_file);
    read_evaluated_unscored_block(rubric_file);
}

void Rubric::print(ofstream &evaluated_rubric)
{
    //header
    evaluated_rubric << "======================== RUBRICA ========================\n";
    evaluated_rubric << student_name << "\n";
    //criteria
    evaluated_rubric << "\nCRITERIOS:\n";
    evaluated_rubric << string(80, '=') << "\n";

    evaluated_rubric << left << setw(6) << "ID" << right << setw(10) << "Base"
            << right << setw(10) << "Obtenido" << string(5, ' ') << "Description\n";
    evaluated_rubric << string(80, '-') << "\n";

    double total_base_score = 0.0, total_achieved_score = 0.0;
    for (const auto &c: criteria)
    {
        c.print(evaluated_rubric);
        total_base_score += c.get_base_score();
        total_achieved_score += c.get_achieved_score();
    }

    evaluated_rubric << string(80, '-') << "\n";
    evaluated_rubric << "Punta obtenido: " << total_achieved_score << "\n\n\n";


    //deductions
    evaluated_rubric << "DESCUENTOS:\n";
    evaluated_rubric << string(80, '=') << "\n";

    evaluated_rubric << left << setw(6) << "ID" << right << setw(10) << "Base"
            << right << setw(10) << "Descontado" << string(5, ' ') << "Description\n";
    evaluated_rubric << string(80, '-') << "\n";

    double total_achieved_deductions = 0.0;
    for (const auto &d: deductions)
    {
        d.print(evaluated_rubric);
        total_achieved_deductions += d.get_achieved_deduct_score();
    }

    evaluated_rubric << string(80, '-') << "\n";
    evaluated_rubric << "Descuentos obtenido: " << total_achieved_deductions << "\n\n\n";

    //observations
    evaluated_rubric << "OBSERVACIONES:\n";
    evaluated_rubric << string(80, '=') << "\n";

    for (const auto &o: observations)
    {
        o.print(evaluated_rubric);
    }

    evaluated_rubric << string(80, '-') << "\n";
    evaluated_rubric << "\n";

    //footer
    evaluated_rubric << "NOTA FINAL: " << total_achieved_score - total_achieved_deductions << endl;
    evaluated_rubric << "========================================================\n";
}
