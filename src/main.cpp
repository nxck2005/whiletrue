#include <atomic>
#include <csignal>
#include <chrono>
#include <string>
#include <thread>
#include <ncurses.h>
#include <cmath>
#include <vector>
#include <fstream>

using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;
using Duration = std::chrono::duration<double>;

// How fast subsequent building prices scale. lower it for an easier game
// LPS should lag behind cost scaling
double COST_SCALE_FACTOR = 1.15;

// for save file consistency
const int VERSION = 1;

struct Building {
    std::string name;
    double basecost;
    double baselps;
    int count;

    double getNextCost() {
        return this->basecost * std::pow(COST_SCALE_FACTOR, this->count);
    }
};

struct Game {
    double linesPerSecond;
    double lines;
    double buffs;
    double baseClickAmt; // starts at 1
    double lpsToClick; // how much % of lps does a click get
    double clickBoostPercent; // for modifiers like 777x etc.
    double lastClickValue = 0;
    double feedbackTimer = 0;

    std::vector<Building> buildings;
    int numBuildings;

    Game(double lps, double b) : linesPerSecond(lps) , buffs(b), lines(0), baseClickAmt(1.0),
        lpsToClick(0), clickBoostPercent(1.0) {
            buildings.push_back({"Ping", 15, 0.1, 0});
            buildings.push_back({"VDB-ICE", 100, 1.0, 0});
            buildings.push_back({"Alt-ICE", 1100, 8.0, 0});
            buildings.push_back({"CANTO", 12000, 47.0, 0});
            buildings.push_back({"EREBUS", 130000, 260.0, 0});
            this->numBuildings = buildings.size();
    }

    void updateLPS() {
        double newlps = 0;
        for (const auto& b : this->buildings) {
            newlps += b.baselps * b.count;
        }
        this->linesPerSecond = newlps;
    }
    void buyBuilding(int index) {
        if (index >= numBuildings) {
            return;
        }
        double cost = buildings[index].getNextCost();
        if (cost <= this->lines) {
            this->buildings[index].count++;
            this->lines -= cost;
            updateLPS();
        }
        return;
    }    
    void runCycle(double deltat) {
        this->lines += this->linesPerSecond * deltat * this->buffs;
        return;
    }
    void registerClick() {
        // get current lps, borrow lpsToClick% to baseClickAmt, and then boost it by clickBoostPercent
        double lps = this->linesPerSecond * this->buffs;
        double lpsContribution = lps * this->lpsToClick;
        double linesToAdd = (this->baseClickAmt + lpsContribution) * this->clickBoostPercent;
        this->lines += linesToAdd;
        this->lastClickValue = linesToAdd;
        this->feedbackTimer = 0.35f; // show an alert for feedbackTimer seconds
    }
    void updateTimers(double dt) {
        if (this->feedbackTimer > 0) this->feedbackTimer -= dt;
    }

    void saveGame() {
        std::ofstream saveFile("save_data.dat");
        if (saveFile.is_open()) {
            saveFile << VERSION << "\n";
            saveFile << this->lines << "\n";
            saveFile << this->buffs << "\n";
            saveFile << this->linesPerSecond << "\n";

            for (const auto& b : this->buildings) {
                saveFile << b.count << "\n";
            }
            saveFile.close();
        }
    }

    void loadGame() {
        std::ifstream saveFile("save_data.dat");
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

        for (auto& b : this->buildings) {
            saveFile >> b.count;
        }

        updateLPS();
        saveFile.close();
    }
};


// Is the game loop running?
std::atomic<bool> keep_running{true};

// if SIGINT is called:
void handle_sigint(int signal) {
    keep_running = false;
}

int main() {
    // Bind SIGINT to our signal handler
    std::signal(SIGINT, handle_sigint);

    // Initialize ncurses
    initscr();              // Start curses mode
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    cbreak();               // Line buffering disabled, Pass on everything to me
    noecho();               // Don't echo() while we getch
    curs_set(0);            // Hide the cursor
    nodelay(stdscr, TRUE);  // Don't wait for user input

    TimePoint lasttime = Clock::now();
    Game game(0, 1.0);
    game.loadGame();
    while (keep_running) {
        int ch;
        while ((ch = getch()) != ERR) {
            if (ch == 'q' || ch == 27) { // esc to quit
                keep_running = false;
            } else if (ch == ' ') { // click
                game.registerClick();
            } else if (ch == 'b') { // bought a buff
                game.buffs += 0.1;
            } else if (ch == 'c') { // bought a cps % of lps
                game.lpsToClick += 0.01;
            } else if (ch == '1') { // BUILDINGS
                game.buyBuilding(0);
            } else if (ch == '2') {
                game.buyBuilding(1);
            } else if (ch == '3') {
                game.buyBuilding(2);
            } else if (ch == '4') {
                game.buyBuilding(3);
            } else if (ch == '5') {
                game.buyBuilding(4);
            } else if (ch == 's') {
                game.saveGame();
            } else if (ch == 'l') {
                game.loadGame();
            }
        }
        TimePoint curtime = Clock::now();
        Duration delta_time = curtime - lasttime;
        lasttime = curtime;

        game.runCycle(delta_time.count());
        game.updateTimers(delta_time.count());

        // Clear the feedback line first so old messages don't stick
        move(1, 2); clrtoeol();
        if (game.feedbackTimer > 0) {
            attron(A_BOLD);
            mvprintw(1, 2, "+++ BREACHED FOR: %.2f DATA +++", game.lastClickValue);
            attroff(A_BOLD);
        }
        mvprintw(3, 2, "BlackWall (SPACE TO BREACH)");
        mvprintw(5, 2, "DATA:            %.2f ", game.lines);
        mvprintw(7, 2, "DATA per second: %.2f", game.linesPerSecond * game.buffs);
        mvprintw(9, 2, "Buff multiplier:  x%.2f", game.buffs);
        mvprintw(11, 2, "DATA/SEC share to BREACH:  %.0f%%", game.lpsToClick * 100);
        attron(A_BOLD);
        mvprintw(24, 2, "QUICKHACKS - COUNT - DATA/SEC - NEXT COST");
        mvprintw(25, 2, "------------------------------------------\n");
        attroff(A_BOLD);
        int baseline = 27;
        for (int i = 0; i < game.buildings.size(); i++) {
            mvprintw(baseline + i, 2, "[%d] %-18s %-6d %-10.2f", 
                     i + 1, 
                     game.buildings[i].name.c_str(), 
                     game.buildings[i].count,
                     game.buildings[i].baselps * game.buildings[i].count);
        
            double cost = game.buildings[i].getNextCost();
            if (game.lines >= cost) {
                attron(COLOR_PAIR(1)); // Green (Affordable)
            } else {
                attron(COLOR_PAIR(2)); // Red (Too expensive)
            }
            mvprintw(baseline + i, 40, "%.2f", cost);
            attroff(COLOR_PAIR(1));
            attroff(COLOR_PAIR(2));
        }
        refresh();

        // check for click and process it too. maybe before the runcycle or after?

        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // Small sleep to prevent 100% CPU usage
    }

    // Deinitialize ncurses
    endwin();

    return 0;
}
