#include "input_handler.hpp"
#include <ncurses.h>

InputHandler::InputHandler() {
    // Basic Actions
    keyMap[' '] = GameAction::BREACH;
    keyMap['b'] = GameAction::BUY_BUFF;
    keyMap['c'] = GameAction::BUY_CLICK_SHARE;
    keyMap['s'] = GameAction::SAVE;
    keyMap['l'] = GameAction::LOAD;
    keyMap['g'] = GameAction::CATCH_CACHE;
    keyMap['q'] = GameAction::QUIT;
    keyMap[27]  = GameAction::QUIT; // ESC
    keyMap[KEY_RESIZE] = GameAction::RESIZE;
    keyMap[KEY_UP] = GameAction::MOVE_UP;
    keyMap[KEY_DOWN] = GameAction::MOVE_DOWN;
    keyMap['\n'] = GameAction::BUY_SELECTED;
    keyMap[KEY_ENTER] = GameAction::BUY_SELECTED;
}

Command InputHandler::handleInput(int ch) const {
    // Check general actions
    auto it = keyMap.find(ch);
    if (it != keyMap.end()) {
        return {it->second, -1};
    }

    return {GameAction::NONE, -1};
}
