#include <atomic>
#include <csignal>
#include <chrono>
#include <thread>
#include "game.hpp"
#include "renderer.hpp"

using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;
using Duration = std::chrono::duration<double>;

std::atomic<bool> keep_running{true};

void handle_sigint(int signal) {
    (void)signal;
    keep_running = false;
}

int main() {
    std::signal(SIGINT, handle_sigint);

    Game game(0, 1.0);
    game.loadGame();

    Renderer renderer;

    TimePoint lasttime = Clock::now();

    while (keep_running) {
        int ch;
        while ((ch = getch()) != ERR) {
            if (ch == 'q' || ch == 27) { keep_running = false; } 
            else if (ch == ' ') { game.registerClick(); } 
            else if (ch == 'b') { game.buyBuff(); } 
            else if (ch == 'c') { game.buyClickShare(); } 
            else if (ch == '0') { game.buyBuilding(0); } 
            else if (ch == '1') { game.buyBuilding(1); } 
            else if (ch == '2') { game.buyBuilding(2); } 
            else if (ch == '3') { game.buyBuilding(3); } 
            else if (ch == '4') { game.buyBuilding(4); }
            else if (ch == '5') { game.buyBuilding(5); }
            else if (ch == '6') { game.buyBuilding(6); }
            else if (ch == '7') { game.buyBuilding(7); }
            else if (ch == '8') { game.buyBuilding(8); }
            else if (ch == '9') { game.buyBuilding(9); }
            else if (ch == '-') { game.buyBuilding(10); }
            else if (ch == '=') { game.buyBuilding(11); } 
            else if (ch == ']') { game.buyBuilding(12); } 
            else if (ch == 's') { game.saveGame(); } 
            else if (ch == 'l') { game.loadGame(); }
            else if (ch == 'g') { game.catchCache(); }
        }

        TimePoint curtime = Clock::now();
        Duration delta_time = curtime - lasttime;
        lasttime = curtime;

        game.runCycle(delta_time.count());
        game.updateTimers(delta_time.count());

        renderer.render(game);

        std::this_thread::sleep_for(std::chrono::milliseconds(16)); 
    }

    game.saveGame();

    return 0;
}
