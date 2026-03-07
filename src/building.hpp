#pragma once

#include <string>
#include <cmath>
#include "constants.hpp"

struct Building {
    int id;
    std::string name;
    double basecost;
    double baselps;
    int count;

    double getNextCost() const {
        return this->basecost * std::pow(COST_SCALE_FACTOR, this->count);
    }
};
