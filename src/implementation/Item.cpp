//
// Created by lmthelex on 21/09/2025.
//

#include "../headers/Item.hpp"

//getters and setters
const string &Item::get_id() const
{
    return id;
}

const string &Item::get_description() const
{
    return description;
}

void Item::set_description(string description_)
{
    description = std::move(description_);
}
