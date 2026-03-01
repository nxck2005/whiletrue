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

    header_win = std::make_unique<Window>(3, maxX, 0, 0);
    stats_win  = std::make_unique<Window>(maxY - 3, maxX / 2, 3, 0);
    shop_win   = std::make_unique<Window>(maxY - 3, maxX - (maxX / 2), 3, maxX / 2);
}

Renderer::~Renderer() {
    endwin();
}

void Renderer::handleResize() {
    getmaxyx(stdscr, maxY, maxX);

    header_win->resize(3, maxX, 0, 0);
    stats_win->resize(maxY - 3, maxX / 2, 3, 0);
    shop_win->resize(maxY - 3, maxX - (maxX / 2), 3, maxX / 2);

    // Clear stdscr to fix ghosting on resize
    clear();
    refresh();
}

void Renderer::render(const Game& game) {
    header_win->clear();
    stats_win->clear();
    shop_win->clear();

    header_win->drawBox();
    stats_win->drawBox();
    shop_win->drawBox();

    drawHeader(game);
    drawStats(game);
    drawShop(game);

    header_win->refresh();
    stats_win->refresh();
    shop_win->refresh();
}

void Renderer::drawHeader(const Game& game) {
    WINDOW* win = header_win->get();
    wattron(win, COLOR_PAIR(3) | A_BOLD);
    mvwprintw(win, 0, 2, " [ SYSTEM STATUS ] ");
    wattroff(win, COLOR_PAIR(3) | A_BOLD);

    if (game.feedbackTimer > 0) {
        wattron(win, A_BOLD);
        mvwprintw(win, 1, 2, "+++ BREACHED FOR: %s DATA +++", Utils::formatNumber(game.lastClickValue).c_str());
        wattroff(win, A_BOLD);
    }
    if (game.autosaveFeedbackTimer > 0) {
        wattron(win, COLOR_PAIR(1) | A_BOLD);
        mvwprintw(win, 1, maxX - 30, "[ SYSTEM: PROGRESS SAVED ]");
        wattroff(win, COLOR_PAIR(1) | A_BOLD);
    }

    if (game.cacheOnScreen) {
        wattron(win, COLOR_PAIR(3) | A_BLINK | A_BOLD);
        mvwprintw(win, 2, 2, " [!] ANOMALOUS SIGNAL DETECTED - PRESS 'g' TO INTERCEPT [!] ");
        wattroff(win, COLOR_PAIR(3) | A_BLINK | A_BOLD);
    }
}

void Renderer::drawStats(const Game& game) {
    WINDOW* win = stats_win->get();
    wattron(win, COLOR_PAIR(3) | A_BOLD);
    mvwprintw(win, 0, 2, " [ TERMINAL ] ");
    wattroff(win, COLOR_PAIR(3) | A_BOLD);

    mvwprintw(win, 2, 2, "TARGET: BlackWall");
    wattron(win, A_REVERSE);
    mvwprintw(win, 3, 2, " PRESS SPACE TO BREACH ");
    wattroff(win, A_REVERSE);
    
    mvwprintw(win, 5, 2, "DATA BANK:       %s", Utils::formatNumber(game.lines).c_str());
    mvwprintw(win, 6, 2, "DATA PER SEC:    %s", Utils::formatNumber(game.linesPerSecond * game.buffs).c_str());

    if (game.cacheBuffDurationTimer > 0) {
        wattron(win, COLOR_PAIR(1) | A_BOLD);
        mvwprintw(win, 7, 2, "%s (%.1fs)", game.activeAlert.c_str(), game.cacheBuffDurationTimer);
        wattroff(win, COLOR_PAIR(1) | A_BOLD);
    }

    mvwprintw(win, 9, 2, "[B] Overclock Multiplier: x%.2f", game.buffs);
    if (game.lines >= game.getBuffCost()) wattron(win, COLOR_PAIR(1)); else wattron(win, COLOR_PAIR(2));
    mvwprintw(win, 10, 6, "Cost: %s DATA", Utils::formatNumber(game.getBuffCost()).c_str());
    wattroff(win, COLOR_PAIR(1)); wattroff(win, COLOR_PAIR(2));

    mvwprintw(win, 12, 2, "[C] Breach DATA/SEC share: %.0f%%", game.lpsToClick * 100);
    if (game.lines >= game.getClickShareCost()) wattron(win, COLOR_PAIR(1)); else wattron(win, COLOR_PAIR(2));
    mvwprintw(win, 13, 6, "Cost: %s DATA", Utils::formatNumber(game.getClickShareCost()).c_str());
    wattroff(win, COLOR_PAIR(1)); wattroff(win, COLOR_PAIR(2));
}

void Renderer::drawShop(const Game& game) {
    WINDOW* win = shop_win->get();
    wattron(win, COLOR_PAIR(3) | A_BOLD);
    mvwprintw(win, 0, 2, " [ BLACK MARKET ] ");
    wattroff(win, COLOR_PAIR(3) | A_BOLD);

    wattron(win, A_BOLD);
    mvwprintw(win, 2, 2, "QUICKHACKS");
    mvwprintw(win, 3, 2, "------------------------------------------");
    wattroff(win, A_BOLD);
    
    for (size_t i = 0; i < game.buildings.size(); i++) {
        int y_pos = 5 + (i * 2);
        mvwprintw(win, y_pos, 2, "[%zu] %-10s (Owned: %d)", 
                 i, game.buildings[i].name.c_str(), game.buildings[i].count);
        
        mvwprintw(win, y_pos + 1, 6, "+%s D/s  |", Utils::formatNumber(game.buildings[i].baselps).c_str());
    
        double cost = game.buildings[i].getNextCost();
        if (game.lines >= cost) {
            wattron(win, COLOR_PAIR(1)); 
        } else {
            wattron(win, COLOR_PAIR(2)); 
        }
        mvwprintw(win, y_pos + 1, 22, " Cost: %s", Utils::formatNumber(cost).c_str());
        wattroff(win, COLOR_PAIR(1));
        wattroff(win, COLOR_PAIR(2));
    }
}
