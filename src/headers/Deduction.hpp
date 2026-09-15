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
    optional<double> achieved_deduct_score;

public:
    //constructor and destructure
    Deduction(string id_, string description_, const double deduct_score_)
        : Item(std::move(id_), std::move(description_))
        , base_deduct_score(deduct_score_)
        , achieved_deduct_score(nullopt) {}
    ~Deduction() override = default;

    //getters and setters
    void set_achieved_deduct_score(double achieved_);
    void clear_achieved_deduct_score();

    double get_base_deduct_score() const;

    double get_achieved_deduct_score() const;
    bool has_achieved_deduct_score() const;

    //methods
    void print(ostream &file) const override;
};
#endif //LAB02_TP_DEDUCTION_HPP
