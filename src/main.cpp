#include <atomic>
#include <csignal>
#include <chrono>
#include <thread>
#include "game.hpp"
#include "renderer.hpp"
#include "input_handler.hpp"

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
    InputHandler inputHandler;

    TimePoint lasttime = Clock::now();

    while (keep_running) {
        int ch;
        while ((ch = getch()) != ERR) {
            Command cmd = inputHandler.handleInput(ch);
            
            switch (cmd.action) {
                case GameAction::QUIT:
                    keep_running = false;
                    break;
                case GameAction::BREACH:
                    game.registerClick();
                    break;
                case GameAction::BUY_BUFF:
                    game.buyBuff();
                    break;
                case GameAction::BUY_CLICK_SHARE:
                    game.buyClickShare();
                    break;
                case GameAction::BUY_BUILDING:
                    game.buyBuilding(cmd.index);
                    break;
                case GameAction::SAVE:
                    game.saveGame();
                    break;
                case GameAction::LOAD:
                    game.loadGame();
                    break;
                case GameAction::CATCH_CACHE:
                    game.catchCache();
                    break;
                case GameAction::RESIZE:
                    renderer.handleResize();
                    break;
                case GameAction::MOVE_UP:
                    renderer.moveSelection(-1, game.buildings.size());
                    break;
                case GameAction::MOVE_DOWN:
                    renderer.moveSelection(1, game.buildings.size());
                    break;
                case GameAction::BUY_SELECTED:
                    game.buyBuilding(renderer.getSelectedIndex());
                    break;
                case GameAction::NONE:
                default:
                    break;
            }
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
