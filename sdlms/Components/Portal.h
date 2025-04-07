#pragma once

#include "AnimatedSprite.h"

#include <string>
#include <variant>
#include <SDL3/SDL.h>

struct Portal
{
    int tm = 0;
    int pt = 0;
    std::u16string pn;
    std::variant<std::u16string, SDL_FPoint> tn;

    // 三段式传送门
    std::vector<AnimatedSprite> a;
    // 三段式传送门下标
    int index = 0;

    Portal() = default;
};
