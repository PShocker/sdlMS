#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "Components/Components.h"
#include <SDL3/SDL.h>

struct GainTip
{
    static void init();
    static void push(const std::u16string &index, int num);

    struct Info
    {
        SDL_Texture *texture;
        unsigned int destory;
    };
    static inline std::vector<Info> gaintips;
};