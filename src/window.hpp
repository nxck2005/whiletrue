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
        wresize(win, h, w);
        mvwin(win, y, x);
    }

    void clear() {
        werase(win);
    }

    void drawBox() {
        box(win, 0, 0);
    }

    void refresh() {
        wrefresh(win);
    }

    WINDOW* get() const { return win; }

private:
    WINDOW* win;
};
