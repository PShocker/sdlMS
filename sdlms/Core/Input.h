#pragma once

#include <SDL3/SDL.h>

struct Input
{
    static const inline bool *state = SDL_GetKeyboardState(nullptr);
};
