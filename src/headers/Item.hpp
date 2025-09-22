//
// Created by lmthelex on 21/09/2025.
//

#ifndef LAB02_TP_ITEM_HPP
#define LAB02_TP_ITEM_HPP

#include "utils.hpp"

class Item
{
protected:
    string id;
    string description;

public:
    //constructor and destructure
    Item(string id, string description)
        : id(std::move(id))
        , description(std::move(description)) {};
    virtual ~Item() = default;

    //getters and setters
    virtual string get_id();
    virtual string get_description();

    //methods
    virtual void print(ofstream &file) const = 0;
};

#endif //LAB02_TP_ITEM_HPP
