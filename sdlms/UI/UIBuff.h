#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "Components/Components.h"
#include <SDL3/SDL.h>

struct UIBuff
{
    int id;
    SDL_FPoint position;

    static void init();
    static void run();

    static inline SDL_Texture *number_cache[10];

    unsigned int destory;
    unsigned int duration;

    unsigned int index;
};
