//
// Created by lmthelex on 21/09/2025.
//

#ifndef LAB02_TP_DEDUCTION_HPP
#define LAB02_TP_DEDUCTION_HPP

#include "Item.hpp"
#include "utils.hpp"

class Deduction : public Item
{
    double base_deduct_score;
    double achieved_deduct_score;

public:
    //constructor and destructure
    Deduction(string id, string description, double deduct_score)
        : Item(std::move(id), std::move(description))
        , base_deduct_score(deduct_score)
        , achieved_deduct_score(0.0) {}

    ~Deduction() override = default;

    //getters and setters
    double set_achieved_deduct_score(double achieved_);

    double get_base_deduct_score() const;

    double get_achieved_deduct_score() const;

    //methods
    void print(ofstream &file) const override;
};
#endif //LAB02_TP_DEDUCTION_HPP
