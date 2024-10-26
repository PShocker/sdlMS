module;

#include <unordered_map>
#include <string>

export module components:mob;

import :animatedsprite;
import :sound;

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
        static const int REMOVE = 4;
        static const int JUMP = 5;
    };
    // STAND,MOVE状态下循环的持续时间
    int tick = 500;
    // 复活时间
    int revive = 0;

    int hp = 100;
    int mp;

    int def = 10;
    int damage = 10;

    int state = State::MOVE;

    std::u16string id;

    std::unordered_map<std::u16string, SoundWarp *> sounds;
};