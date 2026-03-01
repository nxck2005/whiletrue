#pragma once

#include <ncurses.h>
#include <memory>
#include "game.hpp"
#include "window.hpp"

class Renderer {
public:
    Renderer();
    ~Renderer();

    void render(const Game& game);
    void handleResize();

private:
    std::unique_ptr<Window> header_win;
    std::unique_ptr<Window> stats_win;
    std::unique_ptr<Window> shop_win;
    int maxY, maxX;

    void drawHeader(const Game& game);
    void drawStats(const Game& game);
    void drawShop(const Game& game);
};
