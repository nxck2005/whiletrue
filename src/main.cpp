#include <atomic>
#include <csignal>
#include <chrono>
#include <thread>
#include <ncurses.h> // For ncurses UI

using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;
using Duration = std::chrono::duration<double>;

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
    while (keep_running) {
        TimePoint curtime = Clock::now();
        Duration delta_time = curtime - lasttime;
        lasttime = curtime;

        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Small sleep to prevent 100% CPU usage
    }

    // Deinitialize ncurses
    endwin();

    return 0;
}
