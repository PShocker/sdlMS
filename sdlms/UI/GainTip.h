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

    static void init();
    static void run();
    static void push(const std::u16string &id, int num);

    static inline std::list<Wrap> gaintips;
};