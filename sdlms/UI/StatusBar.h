#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "Components/Components.h"
#include <SDL3/SDL.h>

struct StatusBar
{
    static void init();
    static inline SDL_Texture *backgrnd;
    static inline SDL_Texture *backgrnd2;
    static inline SDL_Texture *quickSlot;
    static inline SDL_Texture *chatTarget;
};
