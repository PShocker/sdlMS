#pragma once

#include <string>
#include <list>
#include <unordered_map>
#include "Components/Components.h"
#include <SDL3/SDL.h>

struct GainTip
{
    struct Wrap
    {
        SDL_Texture *texture;
        unsigned int destory;
    };

    static inline SDL_Texture *prefix_cache[7];
    static inline SDL_Texture *bracket_cache[2];
    static inline SDL_Texture *number_cache[10];
    static inline SDL_Texture *plus_cache;

    static void init();
    static void run();
    static void push(const std::u16string &id, int num);

    static inline std::list<Wrap> gaintips;
};