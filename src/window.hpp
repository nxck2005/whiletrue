#pragma once

#include <ncurses.h>

class Window {
public:
    Window(int h, int w, int y, int x) {
        win = newwin(h, w, y, x);
    }

    ~Window() {
        if (win) {
            delwin(win);
        }
    }

    // Disable copy constructor/assignment to prevent double delwin
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    void resize(int h, int w, int y, int x) {
        if (!win) return;
        wresize(win, h, w);
        mvwin(win, y, x);
    }

    void clear() {
        if (!win) return;
        werase(win);
    }

    void drawBox() {
        if (!win) return;
        box(win, 0, 0);
    }

    void refresh() {
        if (!win) return;
        wrefresh(win);
    }

    WINDOW* get() const { return win; }

private:
    WINDOW* win;
};
