#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "Components/Components.h"
#include <SDL3/SDL.h>

struct ToolTip
{
    static void run();
    static void show();
    static void over();
    static void load_maptooltip();
    static void push(SDL_FRect &r, SDL_Texture *texture);

    struct Info
    {
        SDL_FRect r;
        SDL_Texture *texture;
    };
    static inline std::vector<Info> map_tooltips;
    static inline std::vector<Info> tooltips;
};