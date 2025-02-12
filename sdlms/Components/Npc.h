#pragma once

#include "AnimatedSprite.h"

#include <unordered_map>
#include <string>
#include <SDL3/SDL.h>

struct Npc
{
    std::unordered_map<std::u16string, AnimatedSprite> a;

    std::u16string index;

    SDL_FRect rect();
    SDL_FPoint head();
};
