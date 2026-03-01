#pragma once

#include <ncurses.h>
#include <memory>
#include <vector>
#include <string>
#include "game.hpp"
#include "window.hpp"

class Renderer {
public:
    Renderer();
    ~Renderer();

    void render(const Game& game);
    void handleResize();
    void moveSelection(int dir, int max);
    int getSelectedIndex() const { return selectedBuildingIndex; }
    void drawSplashScreen();

private:
    std::unique_ptr<Window> header_win;
    std::unique_ptr<Window> stats_win;
    std::unique_ptr<Window> shop_win;
    int maxY, maxX;
    int selectedBuildingIndex = 0;
    std::vector<std::string> splashBanner;

    void drawHeader(const Game& game);
    void drawStats(const Game& game);
    void drawShop(const Game& game);
};
