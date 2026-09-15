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
    Item(string id_, string description_)
        : id(std::move(id_))
        , description(std::move(description_)) {};
    virtual ~Item() = default;

    //getters and setters
    const string &get_id() const;
    const string &get_description() const;
    void set_description(string description_);

    //methods
    virtual void print(ostream &file) const = 0;
};

#endif //LAB02_TP_ITEM_HPP
