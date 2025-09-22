//
// Created by lmthelex on 21/09/2025.
//

#ifndef LAB02_TP_CRITERION_HPP
#define LAB02_TP_CRITERION_HPP

#include "Item.hpp"
#include "utils.hpp"

class Criterion : public Item
{
protected:
    double base_score;
    double achieved_score;

public:
    //constructor and destructure
    Criterion(string id, string description, const double base_score)
        : Item(std::move(id), std::move(description))
        , base_score(base_score)
        , achieved_score(0.0) {};
    ~Criterion() override = default;

    //getters and setters
    void set_achieved_score(double achieved_);
    double get_base_score() const;
    double get_achieved_score() const;

    //methods
    void print(ofstream &file) const override;
};

#endif //LAB02_TP_CRITERION_HPP
