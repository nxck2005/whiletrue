#include "game.hpp"
#include <fstream>
#include <cstdlib>
#include <random>

Game::Game(double lps, double b) 
    : linesPerSecond(lps), lines(0), buffs(b), baseClickAmt(1.0),
      lpsToClick(0), clickBoostPercent(1.0), lastClickValue(0), feedbackTimer(0),
      autosaveTimer(0), autosaveFeedbackTimer(0), buffsBought(0), clickSharesBought(0),
      cacheActiveTimer(0), cacheBuffDurationTimer(0), cacheOnScreen(false), activeAlert("") {
    
    buildings.push_back({"Ping", 15, 0.1, 0});
    buildings.push_back({"Neural Link", 100, 1.0, 0});
    buildings.push_back({"Coprocessor", 1100, 8.0, 0});
    buildings.push_back({"Grouped Subnet Breach", 12000, 47.0, 0});
    buildings.push_back({"Daemon", 130000, 260.0, 0});
    buildings.push_back({"Deep Dive Port", 1400000, 1400.0, 0});
    buildings.push_back({"Micro-AI", 20000000, 7800.0, 0});
    buildings.push_back({"L.I.L.I.T.H.", 330000000, 44000, 0});
    buildings.push_back({"Bartmoss' Cyberdeck", 5100000000, 260000, 0});
    buildings.push_back({"Project Oracle", 75000000000, 1600000, 0});
    buildings.push_back({"Cynosure Datacore", 1000000000000, 1000000, 0});
    buildings.push_back({"Neural Matrix", 14000000000000, 65000000, 0});
    buildings.push_back({"Alt", 170000000000000, 430000000, 0});
    this->numBuildings = buildings.size();
    this->cacheSpawnTimer = std::rand() % 300;
}

void Game::updateLPS() {
    double newlps = 0;
    for (const auto& b : this->buildings) {
        newlps += b.baselps * b.count;
    }
    this->linesPerSecond = newlps;
}

void Game::buyBuilding(int index) {
    if (index >= numBuildings) {
        return;
    }
    double cost = buildings[index].getNextCost();
    if (cost <= this->lines) {
        this->buildings[index].count++;
        this->lines -= cost;
        updateLPS();
    }
}

double Game::getBuffCost() const {
    return 1000.0 * std::pow(BUFF_COST_SCALE_FACTOR, this->buffsBought);
}

double Game::getClickShareCost() const {
    return 500.0 * std::pow(LPS_TO_CLICK_COST_SCALE_FACTOR, this->clickSharesBought);
}

void Game::buyBuff() {
    double nextCost = this->getBuffCost();
    if (this->lines >= nextCost) {
        this->lines -= nextCost;
        this->buffs += 0.1;
        this->buffsBought++;
        this->updateLPS();
    }
}

void Game::buyClickShare() {
    double nextCost = this->getClickShareCost();
    if (this->lines >= nextCost) {
        this->lines -= nextCost;
        this->lpsToClick += 0.01;
        this->clickSharesBought++;
    }
}

void Game::runCycle(double deltat) {
    this->lines += this->linesPerSecond * deltat * this->buffs;
}

void Game::registerClick() {
    double lps = this->linesPerSecond * this->buffs;
    double lpsContribution = lps * this->lpsToClick;
    double linesToAdd = (this->baseClickAmt + lpsContribution) * this->clickBoostPercent;
    this->lines += linesToAdd;
    this->lastClickValue = linesToAdd;
    this->feedbackTimer = 0.35f;
}

void Game::updateTimers(double dt) {
    if (this->feedbackTimer > 0) this->feedbackTimer -= dt;
    if (this->autosaveFeedbackTimer > 0) this->autosaveFeedbackTimer -= dt;

    this->autosaveTimer += dt;
    if (this->autosaveTimer >= AUTOSAVE_INTERVAL) {
        this->saveGame();
        this->autosaveTimer = 0;
        this->autosaveFeedbackTimer = 2.0;
    }
    if (this->cacheBuffDurationTimer > 0) {
        this->cacheBuffDurationTimer -= dt;
        if (this->cacheBuffDurationTimer <= 0) {
            this->clickBoostPercent = 1.0;
            this->activeAlert = "";
        }
    }
    if (!this->cacheOnScreen) {
        this->cacheSpawnTimer -= dt;
        if (this->cacheSpawnTimer <= 0) {
            this->cacheOnScreen = true;
            this->cacheActiveTimer = 10.0;
        }
    } else {
        this->cacheActiveTimer -= dt;
        if (this->cacheActiveTimer <= 0) {
            this->cacheOnScreen = false;
            this->cacheSpawnTimer = 300 + (std::rand() % 100); 
        }
    }
}

void Game::saveGame() const {
    std::ofstream saveFile(SAVE_FILE_NAME);
    if (saveFile.is_open()) {
        saveFile << VERSION << "\n";
        saveFile << this->lines << "\n";
        saveFile << this->buffs << "\n";
        saveFile << this->linesPerSecond << "\n";
        saveFile << this->buffsBought << "\n";
        saveFile << this->clickSharesBought << "\n";
        saveFile << this->lpsToClick << "\n";

        for (const auto& b : this->buildings) {
            saveFile << b.count << "\n";
        }
        saveFile.close();
    }
}

void Game::loadGame() {
    std::ifstream saveFile(SAVE_FILE_NAME);
    if (!saveFile.is_open()) return;
    int savedver;
    if (!(saveFile >> savedver)) return;

    if (savedver != VERSION) {
        saveFile.close();
        return;
    }

    saveFile >> this->lines;
    saveFile >> this->buffs;
    saveFile >> this->linesPerSecond;
    saveFile >> this->buffsBought;
    saveFile >> this->clickSharesBought;
    saveFile >> this->lpsToClick;

    for (auto& b : this->buildings) {
        saveFile >> b.count;
    }

    updateLPS();
    saveFile.close();
}

void Game::catchCache() {
    if (this->cacheOnScreen) {
        this->cacheOnScreen = false;
        this->cacheSpawnTimer = 45.0 + (std::rand() % 45);
        this->cacheBuffDurationTimer = CACHE_BUFF_DURATION;
        this->clickBoostPercent = CACHE_BUFF_PERCENT;
        this->activeAlert = "BREACH PROTOCOL: 777x DATA MINING FOR 30s!";
        this->feedbackTimer = 2.0; 
    }
}
