//
// Created by lmthelex on 21/09/2025.
//

#ifndef LAB02_TP_OBSERVATION_HPP
#define LAB02_TP_OBSERVATION_HPP

#include "Item.hpp"
#include "utils.hpp"

class Observation : public Item
{
public:
    //constructor and destructure
    Observation(string id_, string description_)
        : Item(std::move(id_), std::move(description_)) {};
    ~Observation() override = default;

    //methods
    void print(ostream &file) const override;

};
#endif //LAB02_TP_OBSERVATION_HPP
