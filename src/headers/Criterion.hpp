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
    optional<double> achieved_score;

public:
    //constructor and destructure
    Criterion(string id_, string description_, const double base_score_)
        : Item(std::move(id_), std::move(description_))
        , base_score(base_score_)
        , achieved_score(nullopt) {};
    ~Criterion() override = default;

    //getters and setters
    void set_achieved_score(double achieved_);
    void clear_achieved_score();
    double get_base_score() const;
    double get_achieved_score() const;
    bool has_achieved_score() const;

    //methods
    void print(ostream &file) const override;
};

#endif //LAB02_TP_CRITERION_HPP
