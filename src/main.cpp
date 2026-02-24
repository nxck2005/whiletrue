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

// -- constants -- //
double COST_SCALE_FACTOR = 1.15;
double BUFF_COST_SCALE_FACTOR = 1.5;
double LPS_TO_CLICK_COST_SCALE_FACTOR = 1.8;
double AUTOSAVE_INTERVAL = 30.0;

// for save file consistency
const int VERSION = 3;

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
    double autosaveTimer = 0;
    double autosaveFeedbackTimer = 0;
    int buffsBought = 0;
    int clickSharesBought = 0;

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

    double getBuffCost() {
        return 1000.0 * std::pow(BUFF_COST_SCALE_FACTOR, this->buffsBought);
    }

    double getClickShareCost() {
        return 500.0 * std::pow(LPS_TO_CLICK_COST_SCALE_FACTOR, this->clickSharesBought);
    }

    void buyBuff() {
        double nextCost = this->getBuffCost();
        if (this->lines >= nextCost) {
            this->lines -= nextCost;
            this->buffs += 0.1;
            this->buffsBought++;
            this->updateLPS();
        }
    }

    void buyClickShare() {
        double nextCost = this->getClickShareCost();
        if (this->lines >= nextCost) {
            this->lines -= nextCost;
            this->lpsToClick += 0.01;
            this->clickSharesBought++;
        }
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
        if (this->autosaveFeedbackTimer > 0) this->autosaveFeedbackTimer -= dt;

        this->autosaveTimer += dt;
        if (this->autosaveTimer >= AUTOSAVE_INTERVAL) {
            this->saveGame();
            this->autosaveTimer = 0;
            this->autosaveFeedbackTimer = 2.0; // show notif for 2 secs
        }
    }

    void saveGame() {
        std::ofstream saveFile("save_data.dat");
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
        saveFile >> this->buffsBought;
        saveFile >> this->clickSharesBought;
        saveFile >> this->lpsToClick;

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
    initscr();              
    start_color();
    use_default_colors();
    init_pair(1, COLOR_GREEN, -1);
    init_pair(2, COLOR_RED, -1);
    init_pair(3, COLOR_CYAN, -1); // Let's add cyan for some Cyberpunk flair
    cbreak();               
    noecho();               
    curs_set(0);            
    nodelay(stdscr, TRUE);  

    // size of the terminal so we can make responsive windows
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);

    // Windows
    WINDOW* header_win = newwin(3, maxX, 0, 0);
    WINDOW* stats_win  = newwin(maxY - 3, maxX / 2, 3, 0);
    WINDOW* shop_win   = newwin(maxY - 3, maxX - (maxX / 2), 3, maxX / 2);

    TimePoint lasttime = Clock::now();
    Game game(0, 1.0);
    game.loadGame();

    while (keep_running) {
        int ch;
        while ((ch = getch()) != ERR) {
            if (ch == 'q' || ch == 27) { keep_running = false; } 
            else if (ch == ' ') { game.registerClick(); } 
            else if (ch == 'b') { game.buyBuff(); } 
            else if (ch == 'c') { game.buyClickShare(); } 
            else if (ch == '1') { game.buyBuilding(0); } 
            else if (ch == '2') { game.buyBuilding(1); } 
            else if (ch == '3') { game.buyBuilding(2); } 
            else if (ch == '4') { game.buyBuilding(3); } 
            else if (ch == '5') { game.buyBuilding(4); } 
            else if (ch == 's') { game.saveGame(); } 
            else if (ch == 'l') { game.loadGame(); }
        }

        // --- GAME LOGIC ---
        TimePoint curtime = Clock::now();
        Duration delta_time = curtime - lasttime;
        lasttime = curtime;

        game.runCycle(delta_time.count());
        game.updateTimers(delta_time.count());

        // --- RENDERING ---

        // clear before redraw
        werase(header_win);
        werase(stats_win);
        werase(shop_win);

        // draw borders
        box(header_win, 0, 0);
        box(stats_win, 0, 0);
        box(shop_win, 0, 0);

        // draw titles
        wattron(stats_win, COLOR_PAIR(3) | A_BOLD);
        mvwprintw(stats_win, 0, 2, " [ TERMINAL ] ");
        wattroff(stats_win, COLOR_PAIR(3) | A_BOLD);

        wattron(shop_win, COLOR_PAIR(3) | A_BOLD);
        mvwprintw(shop_win, 0, 2, " [ BLACK MARKET ] ");
        wattroff(shop_win, COLOR_PAIR(3) | A_BOLD);

        // alerts
        if (game.feedbackTimer > 0) {
            wattron(header_win, A_BOLD);
            mvwprintw(header_win, 1, 2, "+++ BREACHED FOR: %.2f DATA +++", game.lastClickValue);
            wattroff(header_win, A_BOLD);
        }
        if (game.autosaveFeedbackTimer > 0) {
            wattron(header_win, COLOR_PAIR(1) | A_BOLD); 
            mvwprintw(header_win, 1, maxX - 30, "[ SYSTEM: PROGRESS SAVED ]");
            wattroff(header_win, COLOR_PAIR(1) | A_BOLD);
        }

        // stats
        mvwprintw(stats_win, 2, 2, "TARGET: BlackWall");
        wattron(stats_win, A_REVERSE); // Highlight the instruction
        mvwprintw(stats_win, 3, 2, " PRESS SPACE TO BREACH ");
        wattroff(stats_win, A_REVERSE);
        
        mvwprintw(stats_win, 5, 2, "DATA BANK:       %.2f", game.lines);
        mvwprintw(stats_win, 6, 2, "DATA PER SEC:    %.2f", game.linesPerSecond * game.buffs);

        // perm buff stats
        mvwprintw(stats_win, 9, 2, "[B] Overclock Multiplier: x%.2f", game.buffs);
        if (game.lines >= game.getBuffCost()) wattron(stats_win, COLOR_PAIR(1)); else wattron(stats_win, COLOR_PAIR(2));
        mvwprintw(stats_win, 10, 6, "Cost: %.2f DATA", game.getBuffCost());
        wattroff(stats_win, COLOR_PAIR(1)); wattroff(stats_win, COLOR_PAIR(2));

        mvwprintw(stats_win, 12, 2, "[C] Breach DATA/SEC share: %.0f%%", game.lpsToClick * 100);
        if (game.lines >= game.getClickShareCost()) wattron(stats_win, COLOR_PAIR(1)); else wattron(stats_win, COLOR_PAIR(2));
        mvwprintw(stats_win, 13, 6, "Cost: %.2f DATA", game.getClickShareCost());
        wattroff(stats_win, COLOR_PAIR(1)); wattroff(stats_win, COLOR_PAIR(2));


        // shop
        wattron(shop_win, A_BOLD);
        mvwprintw(shop_win, 2, 2, "QUICKHACKS");
        mvwprintw(shop_win, 3, 2, "------------------------------------------");
        wattroff(shop_win, A_BOLD);
        
        for (int i = 0; i < game.buildings.size(); i++) {
            int y_pos = 5 + (i * 2); // Space them out nicely
            mvwprintw(shop_win, y_pos, 2, "[%d] %-10s (Owned: %d)", 
                     i + 1, game.buildings[i].name.c_str(), game.buildings[i].count);
            
            mvwprintw(shop_win, y_pos + 1, 6, "+%.2f D/s  ", game.buildings[i].baselps);
        
            double cost = game.buildings[i].getNextCost();
            if (game.lines >= cost) {
                wattron(shop_win, COLOR_PAIR(1)); 
            } else {
                wattron(shop_win, COLOR_PAIR(2)); 
            }
            mvwprintw(shop_win, y_pos + 1, 22, " Cost: %.2f", cost);
            wattroff(shop_win, COLOR_PAIR(1));
            wattroff(shop_win, COLOR_PAIR(2));
        }

        // refresh all windows
        wrefresh(header_win);
        wrefresh(stats_win);
        wrefresh(shop_win);

        std::this_thread::sleep_for(std::chrono::milliseconds(16)); 
    }

    // cleanup
    game.saveGame();
    delwin(header_win);
    delwin(stats_win);
    delwin(shop_win);
    endwin();

    return 0;
}