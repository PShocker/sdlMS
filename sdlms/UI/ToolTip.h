#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "Components/Components.h"
#include <SDL3/SDL.h>

struct ToolTip
{
    struct Wrap
    {
        SDL_FRect r;
        SDL_Texture *texture;
    };

    static void run();
    static void show();
    static void over();
    static void load_maptooltip();
    static void push(SDL_FRect &r, SDL_Texture *texture);

    static inline std::vector<Wrap> map_tooltips;
    static inline std::vector<Wrap> tooltips;
};