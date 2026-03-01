#pragma once

#include <map>

enum class GameAction {
    NONE,
    BREACH,
    BUY_BUFF,
    BUY_CLICK_SHARE,
    BUY_BUILDING,
    SAVE,
    LOAD,
    CATCH_CACHE,
    RESIZE,
    MOVE_UP,
    MOVE_DOWN,
    BUY_SELECTED,
    QUIT
};

struct Command {
    GameAction action;
    int index; // Used for building index
};

class InputHandler {
    public:
        InputHandler();
        Command handleInput(int ch) const;

    private:
        std::map<int, GameAction> keyMap;
        std::map<int, int> buildingKeys;
};
