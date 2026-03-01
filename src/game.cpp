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
        addLog("SYSTEM: Purchased [" + buildings[index].name + "]");
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
        addLog("SYSTEM: Overclock updated to x" + std::to_string(this->buffs).substr(0,4));
        this->updateLPS();
    }
}

void Game::buyClickShare() {
    double nextCost = this->getClickShareCost();
    if (this->lines >= nextCost) {
        this->lines -= nextCost;
        this->lpsToClick += 0.01;
        this->clickSharesBought++;
        addLog("SYSTEM: Click Share increased to " + std::to_string(int(this->lpsToClick * 100)) + "%");
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

    // Random hex-like packet capture for the log
    char hex[9];
    sprintf(hex, "%08X", (unsigned int)(std::rand() % 0xFFFFFFFF));
    addLog("PKT: [" + std::string(hex) + "] captured (" + std::to_string(int(linesToAdd)) + "B)");
}

void Game::updateTimers(double dt) {
    if (this->feedbackTimer > 0) this->feedbackTimer -= dt;
    if (this->autosaveFeedbackTimer > 0) this->autosaveFeedbackTimer -= dt;

    this->autosaveTimer += dt;
    if (this->autosaveTimer >= AUTOSAVE_INTERVAL) {
        this->saveGame();
        this->autosaveTimer = 0;
        this->autosaveFeedbackTimer = 2.0;
        // In Game::updateTimers we can't easily add a log from saveGame because it's const, 
        // but we can add it here.
        addLog("SYSTEM: Auto-save complete.");
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
    json save_data;
    save_data["version"] = VERSION;
    save_data["lines"] = this->lines;
    save_data["buffs"] = this->buffs;
    save_data["linesPerSecond"] = this->linesPerSecond;
    save_data["buffsBought"] = this->buffsBought;
    save_data["clickSharesBought"] = this->clickSharesBought;
    save_data["lpsToClick"] = this->lpsToClick;

    json buildings_data = json::array();
    for (const auto& b : this->buildings) {
        buildings_data.push_back({
            {"name", b.name},
            {"count", b.count}
        });
    }
    save_data["buildings"] = buildings_data;

    std::ofstream saveFile(SAVE_FILE_NAME);
    if (saveFile.is_open()) {
        saveFile << save_data.dump(4);
        saveFile.close();
    }
}

void Game::loadGame() {
    std::ifstream saveFile(SAVE_FILE_NAME);
    if (!saveFile.is_open()) return;

    try {
        json save_data = json::parse(saveFile);
        
        int savedver = save_data.value("version", 0);
        if (savedver != VERSION) {
            return;
        }

        this->lines = save_data.value("lines", 0.0);
        this->buffs = save_data.value("buffs", 1.0);
        this->linesPerSecond = save_data.value("linesPerSecond", 0.0);
        this->buffsBought = save_data.value("buffsBought", 0);
        this->clickSharesBought = save_data.value("clickSharesBought", 0);
        this->lpsToClick = save_data.value("lpsToClick", 0.0);

        if (save_data.contains("buildings") && save_data["buildings"].is_array()) {
            for (const auto& b_data : save_data["buildings"]) {
                std::string name = b_data.value("name", "");
                int count = b_data.value("count", 0);

                for (auto& b : this->buildings) {
                    if (b.name == name) {
                        b.count = count;
                        break;
                    }
                }
            }
        }

        addLog("SYSTEM: State recovered. Ver " + std::to_string(savedver));
        updateLPS();
    } catch (const std::exception& e) {
        addLog("SYSTEM ERROR: Save data corrupted.");
    }
    
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
        addLog("SIGNAL: Anomalous intercept successful.");
    }
}

void Game::addLog(const std::string& msg) {
    actionLog.push_front(msg);
    if (actionLog.size() > 8) {
        actionLog.pop_back();
    }
}
