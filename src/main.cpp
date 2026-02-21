#include <atomic>
#include <csignal>
#include <chrono>
#include <thread>
#include <ncurses.h> // For ncurses UI

using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;
using Duration = std::chrono::duration<double>;

class Game {
    public:
    double linesPerSecond;
    double lines;
    double buffs;
    double baseClickAmt; // starts at 1
    double lpsToClick; // how much % of lps does a click get
    double clickBoostPercent; // for modifiers like 777x etc.
    double lastClickValue = 0;
    double feedbackTimer = 0;

    Game(double lps, double b) : linesPerSecond(lps) , buffs(b), lines(0), baseClickAmt(1.0),
        lpsToClick(0), clickBoostPercent(1.0) {}
    
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
    cbreak();               // Line buffering disabled, Pass on everything to me
    noecho();               // Don't echo() while we getch
    curs_set(0);            // Hide the cursor
    nodelay(stdscr, TRUE);  // Don't wait for user input

    TimePoint lasttime = Clock::now();
    Game game(1, 1.0);
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
            } else if (ch == 'l') { // increase lines per second
                game.linesPerSecond += 1;
            }
        }
        TimePoint curtime = Clock::now();
        Duration delta_time = curtime - lasttime;
        lasttime = curtime;

        game.runCycle(delta_time.count());
        game.updateTimers(delta_time.count());

        // Clear the feedback line first so old messages don't stick
        move(8, 10); clrtoeol();
        if (game.feedbackTimer > 0) {
            attron(A_BOLD);
            mvprintw(8, 10, "+++ CLICKED FOR: %.2f +++", game.lastClickValue);
            attroff(A_BOLD);
        }
        mvprintw(10, 10, "WHILETRUE : IDLE CLICKER FOR THE TERMINAL");
        mvprintw(12, 10, "Lines:            %.2f ", game.lines);
        mvprintw(14, 10, "Lines per second: %.2f", game.linesPerSecond * game.buffs);
        mvprintw(16, 10, "Buff multiplier:  x%.2f", game.buffs);
        mvprintw(18, 10, "Click LPS share:  %.0f%%", game.lpsToClick * 100);
        refresh();

        // check for click and process it too. maybe before the runcycle or after?

        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // Small sleep to prevent 100% CPU usage
    }

    // Deinitialize ncurses
    endwin();

    return 0;
}
