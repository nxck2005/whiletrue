#pragma once

#include <vector>
#include <string>
#include <deque>
#include "building.hpp"
#include "upgrade.hpp"
#include "constants.hpp"
#include "shop.hpp"

class Game {
public:
    double linesPerSecond;
    double lines;
    double buffs;
    double baseClickAmt;
    double lpsToClick;
    double clickBoostPercent;
    double lastClickValue;
    double lastdeltat; // for rendering eye candy latency
    double feedbackTimer;
    double resetTimer;      // Progress of hard reset
    bool isResetting;       // Whether 'r' is being held
    double lastResetKeyPressTime; // Time since last 'r' char was received
    double autosaveTimer;
    double autosaveFeedbackTimer;
    int buffsBought;
    int clickSharesBought;
    double cacheSpawnTimer;
    double cacheActiveTimer;
    double cacheBuffDurationTimer;
    bool cacheOnScreen;
    std::string activeAlert;
    std::deque<std::string> actionLog;

    std::vector<Building> buildings;
    std::vector<Upgrade> upgrades;
    std::vector<int> visibleUpgradeIndices;
    
    // Multiplier Cache: Stores pre-calculated production multipliers for each building.
    // This avoids O(N) upgrade-list iterations every frame during LPS calculation.
    std::vector<double> cachedBuildingMultipliers; 

    // Global Multiplier Cache: Stores the product of all purchased global software buffs.
    double cachedGlobalMultiplier;                  
    
    int numBuildings;
    Shop selectedShop;

    Game(double lps, double b);

    void loadBuildings();
    void loadUpgrades();
    void updateLPS();
    void updateVisibility();

    // Recalculates the production multipliers for all buildings and the global buff.
    // Should be called whenever a new upgrade is purchased or the game is loaded.
    void recalculateMultipliers(); 
    
    void buyBuilding(int index);
    void buyUpgrade(int index);
    void cycleShop();
    double getBuildingProduction(int index) const;
    int getVisibleUpgradesCount() const;
    double getBuffCost() const;
    double getClickShareCost() const;
    void buyBuff();
    void buyClickShare();
    void runCycle(double deltat);
    void registerClick();
    void updateTimers(double dt);
    void saveGame();
    void loadGame();
    void hardReset(); // New Method
    void catchCache();
    void addLog(const std::string& msg);
};
