#pragma once

#include "AnimatedSprite.h"
#include "entt/entt.hpp"
#include "Core/Core.h"

struct Pet
{
    std::u16string id;
    entt::entity owner = entt::null;

    std::unordered_map<std::u16string, AnimatedSprite> a;

    std::u16string index = u"jump";

    struct State
    {
        static const int STAND = 0;
        static const int MOVE = 1;
        static const int JUMP = 2;
        static const int CLIMB = 3;
    };

    int state = State::JUMP;
};