#pragma once

#include <unordered_map>

enum class GameAction {
    QUIT,
    BREACH,
    BUY_BUFF,
    BUY_CLICK_SHARE,
    SAVE,
    LOAD,
    CATCH_CACHE,
    RESIZE,
    MOVE_UP,
    MOVE_DOWN,
    BUY_SELECTED,
    CYCLE_SHOP,
    RESET_PRESS,
    NONE
};

struct Command {
    GameAction action;
    int index; 
};

class InputHandler {
public:
    InputHandler();
    Command handleInput(int ch) const;

private:
    std::unordered_map<int, GameAction> keyMap;
};
