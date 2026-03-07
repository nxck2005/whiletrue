#pragma once
#include <string>
#include <vector>
#include <unordered_map>

struct Upgrade {
    int id;
    std::string name;
    std::string desc;
    double cost;
    bool purchased = false;
    bool visible = false;
    bool alwaysVisible = false;
    double globalMultiplier = 1.0;

    // Building ID -> Required Count
    std::unordered_map<int, int> neededCountsBuildings;

    // Building ID -> Multiplier (e.g., 2.0 = double production)
    std::unordered_map<int, double> resultantBuffs;
};
