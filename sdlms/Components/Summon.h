#pragma once

#include "AnimatedSprite.h"
#include "entt/entt.hpp"
#include "Core/Core.h"

struct Summon
{
    std::u16string id;
    entt::entity owner = entt::null;

    std::unordered_map<std::u16string, AnimatedSprite> a;

    std::u16string index;

    Attack atk;

    // 状态机
    struct State
    {
        static const int INIT = 0;
        static const int STAND = 1;
        static const int MOVE = 2;
        static const int DIE = 3;
        static const int FLY = 4;
        static const int ATTACK = 5;
        static const int REMOVE = 6;
    };

    int state = State::INIT;

    // 摧毁时间
    unsigned int destory = Window::dt_now + 30000;
};