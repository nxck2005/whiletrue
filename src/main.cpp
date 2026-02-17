#include <atomic>
#include <csignal>
#include <chrono>
#include <thread>
#include <ncurses.h> // For ncurses UI

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
    // TODO: Add proper ncurses initialization and error handling
    initscr();              // Start curses mode
    cbreak();               // Line buffering disabled, Pass on everything to me
    noecho();               // Don't echo() while we getch
    curs_set(0);            // Hide the cursor
    nodelay(stdscr, TRUE);  // Don't wait for user input

    // Game loop
    auto last_frame_time = std::chrono::steady_clock::now();
    while (keep_running) {
        auto current_frame_time = std::chrono::steady_clock::now();
        std::chrono::duration<double> delta_time = current_frame_time - last_frame_time;
        last_frame_time = current_frame_time;

        // TODO: Implement game logic, state mutation, input handling, and rendering
        // Example: total_loc += (rate_per_second * delta_time.count());

        // Placeholder for now
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Small sleep to prevent 100% CPU usage
    }

    // Deinitialize ncurses
    endwin();

    return 0;
}
