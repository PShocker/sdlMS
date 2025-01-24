#pragma once

#include "AnimatedSprite.h"
#include <SDL3/SDL.h>

struct Trap
{
    int damage = 5;

    Trap(int damage);

    SDL_FRect rect(AnimatedSprite *aspr);
};
