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

    // Building Keys
    buildingKeys['0'] = 0;
    buildingKeys['1'] = 1;
    buildingKeys['2'] = 2;
    buildingKeys['3'] = 3;
    buildingKeys['4'] = 4;
    buildingKeys['5'] = 5;
    buildingKeys['6'] = 6;
    buildingKeys['7'] = 7;
    buildingKeys['8'] = 8;
    buildingKeys['9'] = 9;
    buildingKeys['-'] = 10;
    buildingKeys['='] = 11;
    buildingKeys[']'] = 12;
}

Command InputHandler::handleInput(int ch) const {
    // Check general actions
    auto it = keyMap.find(ch);
    if (it != keyMap.end()) {
        return {it->second, -1};
    }

    // Check building purchases
    auto bit = buildingKeys.find(ch);
    if (bit != buildingKeys.end()) {
        return {GameAction::BUY_BUILDING, bit->second};
    }

    return {GameAction::NONE, -1};
}
