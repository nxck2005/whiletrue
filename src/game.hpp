#pragma once

#include <vector>
#include <string>
#include "building.hpp"
#include "constants.hpp"

class Game {
public:
    double linesPerSecond;
    double lines;
    double buffs;
    double baseClickAmt;
    double lpsToClick;
    double clickBoostPercent;
    double lastClickValue;
    double feedbackTimer;
    double autosaveTimer;
    double autosaveFeedbackTimer;
    int buffsBought;
    int clickSharesBought;
    double cacheSpawnTimer;
    double cacheActiveTimer;
    double cacheBuffDurationTimer;
    bool cacheOnScreen;
    std::string activeAlert;

    std::vector<Building> buildings;
    int numBuildings;

    Game(double lps, double b);

    void updateLPS();
    void buyBuilding(int index);
    double getBuffCost() const;
    double getClickShareCost() const;
    void buyBuff();
    void buyClickShare();
    void runCycle(double deltat);
    void registerClick();
    void updateTimers(double dt);
    void saveGame() const;
    void loadGame();
    void catchCache();
};
