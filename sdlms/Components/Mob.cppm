module;

#include <unordered_map>
#include <string>

export module components:mob;

import :animatedsprite;

export struct Mob
{
    std::unordered_map<std::u16string, AnimatedSprite *> a;

    std::u16string index;

    // 状态机
    struct State
    {
        static const int STAND = 0;
        static const int MOVE = 1;
        static const int HIT = 2;
        static const int DIE = 3;
    };
    // STAND,MOVE状态下循环的持续时间
    int tick = 200;

    int state = State::MOVE;
};