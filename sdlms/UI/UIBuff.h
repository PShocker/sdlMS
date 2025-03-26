#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "Components/Components.h"
#include <SDL3/SDL.h>

struct UIBuff
{
    std::u16string id;
    SDL_FPoint position;

    static void run();

    unsigned int destory;
    unsigned int duration;

    unsigned int index;
};
