#include "renderer.hpp"
#include "utils.hpp"

Renderer::Renderer() {
    initscr();
    start_color();
    use_default_colors();
    init_pair(1, COLOR_GREEN, -1);
    init_pair(2, COLOR_RED, -1);
    init_pair(3, COLOR_CYAN, -1);
    cbreak();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);

    getmaxyx(stdscr, maxY, maxX);

    header_win = newwin(3, maxX, 0, 0);
    stats_win  = newwin(maxY - 3, maxX / 2, 3, 0);
    shop_win   = newwin(maxY - 3, maxX - (maxX / 2), 3, maxX / 2);
}

Renderer::~Renderer() {
    delwin(header_win);
    delwin(stats_win);
    delwin(shop_win);
    endwin();
}

void Renderer::render(const Game& game) {
    werase(header_win);
    werase(stats_win);
    werase(shop_win);

    box(header_win, 0, 0);
    box(stats_win, 0, 0);
    box(shop_win, 0, 0);

    drawHeader(game);
    drawStats(game);
    drawShop(game);

    wrefresh(header_win);
    wrefresh(stats_win);
    wrefresh(shop_win);
}

void Renderer::drawHeader(const Game& game) {
    wattron(header_win, COLOR_PAIR(3) | A_BOLD);
    mvwprintw(header_win, 0, 2, " [ SYSTEM STATUS ] ");
    wattroff(header_win, COLOR_PAIR(3) | A_BOLD);

    if (game.feedbackTimer > 0) {
        wattron(header_win, A_BOLD);
        mvwprintw(header_win, 1, 2, "+++ BREACHED FOR: %s DATA +++", Utils::formatNumber(game.lastClickValue).c_str());
        wattroff(header_win, A_BOLD);
    }
    if (game.autosaveFeedbackTimer > 0) {
        wattron(header_win, COLOR_PAIR(1) | A_BOLD);
        mvwprintw(header_win, 1, maxX - 30, "[ SYSTEM: PROGRESS SAVED ]");
        wattroff(header_win, COLOR_PAIR(1) | A_BOLD);
    }

    if (game.cacheOnScreen) {
        wattron(header_win, COLOR_PAIR(3) | A_BLINK | A_BOLD);
        mvwprintw(header_win, 2, 2, " [!] ANOMALOUS SIGNAL DETECTED - PRESS 'g' TO INTERCEPT [!] ");
        wattroff(header_win, COLOR_PAIR(3) | A_BLINK | A_BOLD);
    }
}

void Renderer::drawStats(const Game& game) {
    wattron(stats_win, COLOR_PAIR(3) | A_BOLD);
    mvwprintw(stats_win, 0, 2, " [ TERMINAL ] ");
    wattroff(stats_win, COLOR_PAIR(3) | A_BOLD);

    mvwprintw(stats_win, 2, 2, "TARGET: BlackWall");
    wattron(stats_win, A_REVERSE);
    mvwprintw(stats_win, 3, 2, " PRESS SPACE TO BREACH ");
    wattroff(stats_win, A_REVERSE);
    
    mvwprintw(stats_win, 5, 2, "DATA BANK:       %s", Utils::formatNumber(game.lines).c_str());
    mvwprintw(stats_win, 6, 2, "DATA PER SEC:    %s", Utils::formatNumber(game.linesPerSecond * game.buffs).c_str());

    if (game.cacheBuffDurationTimer > 0) {
        wattron(stats_win, COLOR_PAIR(1) | A_BOLD);
        mvwprintw(stats_win, 7, 2, "%s (%.1fs)", game.activeAlert.c_str(), game.cacheBuffDurationTimer);
        wattroff(stats_win, COLOR_PAIR(1) | A_BOLD);
    }

    mvwprintw(stats_win, 9, 2, "[B] Overclock Multiplier: x%.2f", game.buffs);
    if (game.lines >= game.getBuffCost()) wattron(stats_win, COLOR_PAIR(1)); else wattron(stats_win, COLOR_PAIR(2));
    mvwprintw(stats_win, 10, 6, "Cost: %s DATA", Utils::formatNumber(game.getBuffCost()).c_str());
    wattroff(stats_win, COLOR_PAIR(1)); wattroff(stats_win, COLOR_PAIR(2));

    mvwprintw(stats_win, 12, 2, "[C] Breach DATA/SEC share: %.0f%%", game.lpsToClick * 100);
    if (game.lines >= game.getClickShareCost()) wattron(stats_win, COLOR_PAIR(1)); else wattron(stats_win, COLOR_PAIR(2));
    mvwprintw(stats_win, 13, 6, "Cost: %s DATA", Utils::formatNumber(game.getClickShareCost()).c_str());
    wattroff(stats_win, COLOR_PAIR(1)); wattroff(stats_win, COLOR_PAIR(2));
}

void Renderer::drawShop(const Game& game) {
    wattron(shop_win, COLOR_PAIR(3) | A_BOLD);
    mvwprintw(shop_win, 0, 2, " [ BLACK MARKET ] ");
    wattroff(shop_win, COLOR_PAIR(3) | A_BOLD);

    wattron(shop_win, A_BOLD);
    mvwprintw(shop_win, 2, 2, "QUICKHACKS");
    mvwprintw(shop_win, 3, 2, "------------------------------------------");
    wattroff(shop_win, A_BOLD);
    
    for (size_t i = 0; i < game.buildings.size(); i++) {
        int y_pos = 5 + (i * 2);
        mvwprintw(shop_win, y_pos, 2, "[%zu] %-10s (Owned: %d)", 
                 i, game.buildings[i].name.c_str(), game.buildings[i].count);
        
        mvwprintw(shop_win, y_pos + 1, 6, "+%s D/s  |", Utils::formatNumber(game.buildings[i].baselps).c_str());
    
        double cost = game.buildings[i].getNextCost();
        if (game.lines >= cost) {
            wattron(shop_win, COLOR_PAIR(1)); 
        } else {
            wattron(shop_win, COLOR_PAIR(2)); 
        }
        mvwprintw(shop_win, y_pos + 1, 22, " Cost: %s", Utils::formatNumber(cost).c_str());
        wattroff(shop_win, COLOR_PAIR(1));
        wattroff(shop_win, COLOR_PAIR(2));
    }
}
