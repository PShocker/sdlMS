#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "Components/Components.h"
#include <SDL3/SDL.h>

struct ToolTip
{
    static void run();
    static void push(SDL_FRect &r, SDL_Texture *texture);

    static inline std::vector<std::pair<SDL_FRect, SDL_Texture *>> tooltips;
};