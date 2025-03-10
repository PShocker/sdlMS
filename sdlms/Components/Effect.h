#pragma once
#include "Transform.h"
#include "AnimatedSprite.h"
#include "Core/Core.h"

#include <string>

struct Effect
{
    struct Info
    {
        Transform *tr;
        AnimatedSprite aspr;
        unsigned int delay = Window::dt_now;
    };

    std::list<Info> effect_list;
    Effect() = default;

    static AnimatedSpriteWarp *load(const std::u16string &path);

    static void push(Effect *eff, AnimatedSpriteWarp *asprw, std::optional<SDL_FPoint> &p, int flip);
};
