#include "renderer.hpp"
#include "utils.hpp"
#include "json.hpp"
#include <fstream>
#include <clocale>

using json = nlohmann::json;

Renderer::Renderer() {
    std::setlocale(LC_ALL, "");
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
    keypad(stdscr, TRUE);

    getmaxyx(stdscr, maxY, maxX);

    // Load assets
    std::string assetPath = "./data/assets.json";
    std::ifstream f(assetPath);
    if (f.is_open()) {
        try {
            json data = json::parse(f);
            if (data.contains("splash_banner") && data["splash_banner"].is_array()) {
                splashBanner.clear();
                for (const auto& line : data["splash_banner"]) {
                    splashBanner.push_back(line.get<std::string>());
                }
            }
        } catch (...) { /* ignore */ }
    }

    header_win = std::make_unique<Window>(3, maxX, 0, 0);
    stats_win  = std::make_unique<Window>(maxY - 3, maxX / 2, 3, 0);
    shop_win   = std::make_unique<Window>(maxY - 3, maxX - (maxX / 2), 3, maxX / 2);
}

void Renderer::drawSplashScreen() {
    bool waiting = true;
    nodelay(stdscr, FALSE);
    
    while (waiting) {
        erase();
        getmaxyx(stdscr, maxY, maxX);
        
        int bannerHeight = splashBanner.size();
        int maxWidth = 0;
        for (const auto& line : splashBanner) {
            if ((int)line.length() > maxWidth) maxWidth = line.length();
        }

        int startY = (maxY / 2) - (bannerHeight / 2) - 2;
        int bannerStartX = (maxX / 2) - (maxWidth / 2);
        
        if (bannerHeight > 0) {
            attron(COLOR_PAIR(3) | A_BOLD);
            for (int i = 0; i < bannerHeight; ++i) {
                mvprintw(startY + i, bannerStartX, "%s", splashBanner[i].c_str());
            }
            attroff(COLOR_PAIR(3) | A_BOLD);
        }

        attron(A_BLINK | A_BOLD);
        std::string msg = "--- PRESS ANY KEY TO INITIALIZE BREACH ---";
        mvprintw(startY + bannerHeight + 4, (maxX / 2) - (msg.length() / 2), "%s", msg.c_str());
        attroff(A_BLINK | A_BOLD);
        
        refresh();

        int ch = getch();
        if (ch != KEY_RESIZE) {
            waiting = false;
        } else {
            // Re-initialize windows or state if needed on resize during splash
            handleResize();
        }
    }
    
    nodelay(stdscr, TRUE);
}

Renderer::~Renderer() {
    // Explicitly destroy windows BEFORE endwin()
    header_win.reset();
    stats_win.reset();
    shop_win.reset();
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

    // Decorative Metadata
    mvwprintw(win, 1, 2, "CONN: SECURE-AES-256");
    mvwprintw(win, 1, 25, "LATENCY: %dms", (std::rand() % 10) + 5);
    
    // CPU Load Bar
    mvwprintw(win, 1, 45, "CPU: [");
    int load = (int)(game.linesPerSecond > 0 ? 8 : 1) + (std::rand() % 3);
    if (game.feedbackTimer > 0) load = 12; // Spike on click
    for (int i = 0; i < 15; i++) {
        if (i < load) waddch(win, '|' | COLOR_PAIR(1));
        else waddch(win, '.' | A_DIM);
    }
    waddch(win, ']');

    if (game.feedbackTimer > 0) {
        wattron(win, A_BOLD);
        mvwprintw(win, 2, 2, "+++ BREACHED FOR: %s DATA +++", Utils::formatNumber(game.lastClickValue).c_str());
        wattroff(win, A_BOLD);
    }
    if (game.autosaveFeedbackTimer > 0) {
        wattron(win, COLOR_PAIR(1) | A_BOLD);
        mvwprintw(win, 1, maxX - 30, "[ SYSTEM: PROGRESS SAVED ]");
        wattroff(win, COLOR_PAIR(1) | A_BOLD);
    }

    if (game.cacheOnScreen) {
        wattron(win, COLOR_PAIR(3) | A_BLINK | A_BOLD);
        mvwprintw(win, 2, maxX - 65, " [!] ANOMALOUS SIGNAL DETECTED - PRESS 'g' TO INTERCEPT [!] ");
        wattroff(win, COLOR_PAIR(3) | A_BLINK | A_BOLD);
    }
}

void Renderer::drawStats(const Game& game) {
    WINDOW* win = stats_win->get();
    wattron(win, COLOR_PAIR(3) | A_BOLD);
    mvwprintw(win, 0, 2, " [ TERMINAL ] ");
    wattroff(win, COLOR_PAIR(3) | A_BOLD);

    mvwprintw(win, 2, 2, "TARGET: BlackWall_Mainframe_Node7");
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

    // Data Stream Log
    int startLine = 15;
    wattron(win, A_DIM | A_BOLD);
    mvwprintw(win, startLine++, 2, "--- LOG_STREAM_INITIALIZED ---");
    wattroff(win, A_DIM | A_BOLD);
    
    for (size_t i = 0; i < game.actionLog.size(); i++) {
        mvwprintw(win, startLine + i, 2, "> %s", game.actionLog[i].c_str());
    }
}

void Renderer::moveSelection(int dir, int max) {
    selectedBuildingIndex += dir;
    if (selectedBuildingIndex < 0) selectedBuildingIndex = 0;
    if (selectedBuildingIndex >= max) selectedBuildingIndex = max - 1;
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

    // Calculate how many items can be displayed
    // Start at y=5, each item is 2 lines. Box and title use some space.
    int winHeight, winWidth;
    getmaxyx(win, winHeight, winWidth);
    (void)winWidth;
    int displayableCount = (winHeight - 6) / 2;
    if (displayableCount < 1) displayableCount = 1;

    // Static scrolling offset calculation to keep selection in view
    static int scrollOffset = 0;
    if (selectedBuildingIndex < scrollOffset) {
        scrollOffset = selectedBuildingIndex;
    } else if (selectedBuildingIndex >= scrollOffset + displayableCount) {
        scrollOffset = selectedBuildingIndex - displayableCount + 1;
    }

    int endIndex = scrollOffset + displayableCount;
    if (endIndex > (int)game.buildings.size()) endIndex = game.buildings.size();

    for (int i = scrollOffset; i < endIndex; i++) {
        int relativeIdx = i - scrollOffset;
        int y_pos = 5 + (relativeIdx * 2);

        bool isSelected = (i == selectedBuildingIndex);
        if (isSelected) wattron(win, A_BOLD);

        if (isSelected) {
            mvwprintw(win, y_pos, 2, "[%zu] [[ %-10s ]] (Owned: %d)", 
                     (size_t)i, game.buildings[i].name.c_str(), game.buildings[i].count);
        } else {
            mvwprintw(win, y_pos, 2, "[%zu]    %-10s    (Owned: %d)", 
                     (size_t)i, game.buildings[i].name.c_str(), game.buildings[i].count);
        }

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

        if (isSelected) {
            wattroff(win, A_BOLD);
        }
    }
    if (scrollOffset > 0) mvwprintw(win, 4, winWidth - 3, "^");
    if (endIndex < (int)game.buildings.size()) mvwprintw(win, winHeight - 2, winWidth - 3, "v");
}
