#include "game.hpp"
#include <fstream>
#include <cstdlib>
#include <random>
#include "json.hpp"
#include "utils.hpp"

using json = nlohmann::json;

Game::Game(double lps, double b) 
    : linesPerSecond(lps), lines(0), buffs(b), baseClickAmt(1.0),
      lpsToClick(0), clickBoostPercent(1.0), lastClickValue(0), feedbackTimer(0),
      autosaveTimer(0), autosaveFeedbackTimer(0), buffsBought(0), clickSharesBought(0),
      cacheActiveTimer(0), cacheBuffDurationTimer(0), cacheOnScreen(false), activeAlert(""),
      selectedShop(Shop::BUILDINGS) {
    
    loadBuildings();
    loadUpgrades();
    this->cacheSpawnTimer = std::rand() % 300;
}

void Game::cycleShop() {
    if (selectedShop == Shop::BUILDINGS) {
        selectedShop = Shop::UPGRADES;
        addLog("UI: Switched to SOFTWARE SUITE");
    } else {
        selectedShop = Shop::BUILDINGS;
        addLog("UI: Switched to BLACK MARKET");
    }
}

void Game::loadBuildings() {
    std::string path = Utils::getDataPath("buildings.json");
    std::ifstream f(path);
    if (f.is_open()) {
        try {
            json data = json::parse(f);
            this->buildings.clear();
            for (const auto& item : data) {
                this->buildings.push_back({
                    item.at("id").get<int>(),
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
        buildings.push_back({0, "BUILDING LOAD ERROR", 404, 0.1, 0});
    }

    this->numBuildings = buildings.size();
}

void Game::loadUpgrades() {
    std::string path = Utils::getDataPath("upgrades.json");
    std::ifstream f(path);
    if (f.is_open()) {
        try {
            json data = json::parse(f);
            this->upgrades.clear();
            for (const auto& item : data) {
                Upgrade u;
                u.id = item.at("id").get<int>();
                u.name = item.at("name").get<std::string>();
                u.desc = item.at("desc").get<std::string>();
                u.cost = item.at("cost").get<double>();
                u.purchased = false;

                if (item.contains("neededCountsBuildings")) {
                    for (auto& el : item["neededCountsBuildings"].items()) {
                        u.neededCountsBuildings[std::stoi(el.key())] = el.value().get<int>();
                    }
                }

                if (item.contains("resultantBuffs")) {
                    for (auto& el : item["resultantBuffs"].items()) {
                        u.resultantBuffs[std::stoi(el.key())] = el.value().get<double>();
                    }
                }

                this->upgrades.push_back(u);
            }
        } catch (const std::exception& e) {
            // ignore
        }
    }
}

void Game::updateLPS() {
    double newlps = 0;
    
    // Calculate LPS for each building
    for (const auto& b : this->buildings) {
        double multiplier = 1.0;
        
        // Apply multipliers from purchased upgrades
        for (const auto& u : this->upgrades) {
            if (u.purchased) {
                auto it = u.resultantBuffs.find(b.id);
                if (it != u.resultantBuffs.end()) {
                    multiplier *= it->second;
                }
            }
        }
        
        newlps += (b.baselps * multiplier) * b.count;
    }
    this->linesPerSecond = newlps;
}

void Game::buyBuilding(int index) {
    if (index < 0 || index >= numBuildings) {
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

void Game::buyUpgrade(int index) {
    if (index < 0 || index >= (int)upgrades.size()) return;
    
    Upgrade& u = upgrades[index];
    if (u.purchased) return;

    // Check cost
    if (this->lines < u.cost) return;

    // Check requirements
    for (const auto& req : u.neededCountsBuildings) {
        bool met = false;
        for (const auto& b : this->buildings) {
            if (b.id == req.first && b.count >= req.second) {
                met = true;
                break;
            }
        }
        if (!met) return;
    }

    // Purchase
    this->lines -= u.cost;
    u.purchased = true;
    addLog("SOFTWARE: Installed [" + u.name + "]");
    updateLPS();
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
    addLog("PKT: [" + std::string(hex) + "] captured (" + Utils::formatNumber((linesToAdd)) + " DATA)");
}

void Game::updateTimers(double dt) {
    this->lastdeltat = dt;
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

void Game::saveGame() {
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
            {"id", b.id},
            {"count", b.count}
        });
    }
    save_data["buildings"] = buildings_data;

    json upgrades_data = json::array();
    for (const auto& u : this->upgrades) {
        if (u.purchased) {
            upgrades_data.push_back(u.id);
        }
    }
    save_data["upgrades"] = upgrades_data;

    std::ofstream saveFile(Utils::getSavePath());
    if (saveFile.is_open()) {
        saveFile << save_data.dump(4);
        saveFile.close();
    }
    addLog("SYSTEM: Saved state.");
}

void Game::loadGame() {
    std::ifstream saveFile(Utils::getSavePath());
    if (!saveFile.is_open()) return;

    try {
        json save_data = json::parse(saveFile);
        
        int savedver = save_data.value("version", 0);
        if (savedver != VERSION) {
            addLog("SYSTEM RESET: Save data version mismatch! Reset to defaults.");
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
                int id = b_data.value("id", -1);
                int count = b_data.value("count", 0);

                for (auto& b : this->buildings) {
                    if (b.id == id) {
                        b.count = count;
                        break;
                    }
                }
            }
        }

        if (save_data.contains("upgrades") && save_data["upgrades"].is_array()) {
            for (const auto& u_id : save_data["upgrades"]) {
                int id = u_id.get<int>();
                for (auto& u : this->upgrades) {
                    if (u.id == id) {
                        u.purchased = true;
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
    if (actionLog.size() > EYE_CANDY_LOG_SIZE) {
        actionLog.pop_back();
    }
}
