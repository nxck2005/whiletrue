#pragma once

#include <ncurses.h>
#include "game.hpp"

class Renderer {
public:
    Renderer();
    ~Renderer();

    void render(const Game& game);

private:
    WINDOW* header_win;
    WINDOW* stats_win;
    WINDOW* shop_win;
    int maxY, maxX;

    void drawHeader(const Game& game);
    void drawStats(const Game& game);
    void drawShop(const Game& game);
};
