#pragma once

#include <string>
#include <list>
#include <unordered_map>
#include "Components/Components.h"
#include <SDL3/SDL.h>

struct GainTip
{
    static void init();
    static void run();
    static void push(const std::u16string &index, int num);

    static SDL_Texture *load_num_texture(int num);

    struct Info
    {
        SDL_Texture *texture;
        unsigned int destory;
    };

    static inline std::list<Info> gaintips;
};