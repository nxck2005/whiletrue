#include "game.hpp"
#include <fstream>
#include <cstdlib>
#include <random>
#include "json.hpp"

using json = nlohmann::json;

Game::Game(double lps, double b) 
    : linesPerSecond(lps), lines(0), buffs(b), baseClickAmt(1.0),
      lpsToClick(0), clickBoostPercent(1.0), lastClickValue(0), feedbackTimer(0),
      autosaveTimer(0), autosaveFeedbackTimer(0), buffsBought(0), clickSharesBought(0),
      cacheActiveTimer(0), cacheBuffDurationTimer(0), cacheOnScreen(false), activeAlert("") {
    
    loadBuildings();
    this->cacheSpawnTimer = std::rand() % 300;
}

void Game::loadBuildings() {
    std::ifstream f("./data/buildings.json");
    if (f.is_open()) {
        try {
            json data = json::parse(f);
            this->buildings.clear();
            for (const auto& item : data) {
                this->buildings.push_back({
                    item.at("name").get<std::string>(),
                    item.at("basecost").get<double>(),
                    item.at("baselps").get<double>(),
                    0
                });
            }
        } catch (const std::exception& e) {
            // fallback if json is malformed
        }
    }
    
    // Fallback if file not found or empty
    if (this->buildings.empty()) {
        buildings.push_back({"BUILDING LOAD ERROR", 404, 0.1, 0});
    }

    this->numBuildings = buildings.size();
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

    for (size_t i = 0; i < this->buildings.size(); i++) {
        int count;
        if (saveFile >> count) {
            this->buildings[i].count = count;
        }
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
