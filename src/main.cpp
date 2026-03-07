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

    // Display splash screen and wait for input
    renderer.drawSplashScreen();

    TimePoint lasttime = Clock::now();

    while (keep_running) {
        int ch;
        while ((ch = getch()) != ERR) {
            Command cmd = inputHandler.handleInput(ch);
            
            switch (cmd.action) {
                case GameAction::QUIT:
                    keep_running = false;
                    break;
                case GameAction::RESET_PRESS:
                    game.isResetting = true;
                    game.lastResetKeyPressTime = 0; // Reset the cooldown
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
                case GameAction::CYCLE_SHOP:
                    game.cycleShop();
                    break;
                case GameAction::MOVE_UP:
                    if (game.selectedShop == Shop::BUILDINGS)
                        renderer.moveSelection(-1, game.buildings.size(), game.selectedShop);
                    else
                        renderer.moveSelection(-1, game.getVisibleUpgradesCount(), game.selectedShop);
                    break;
                case GameAction::MOVE_DOWN:
                    if (game.selectedShop == Shop::BUILDINGS)
                        renderer.moveSelection(1, game.buildings.size(), game.selectedShop);
                    else
                        renderer.moveSelection(1, game.getVisibleUpgradesCount(), game.selectedShop);
                    break;
                case GameAction::BUY_SELECTED:
                    if (game.selectedShop == Shop::BUILDINGS)
                        game.buyBuilding(renderer.getSelectedIndex());
                    else
                        game.buyUpgrade(renderer.getSelectedUpgradeIndex(game));
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
