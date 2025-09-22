//
// Created by lmthelex on 21/09/2025.
//

#include "..\headers\Observation.hpp"

//methods
void Observation::print(ofstream &file) const
{
    file << string(5, ' ') << description << "\n";
}
