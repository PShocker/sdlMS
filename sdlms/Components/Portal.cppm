module;
#include <string>
#include <unordered_map>
#include <SDL3/SDL.h>

export module components:portal;

import :transform;

export struct Portal
{
    int tm = 0;
    int pt = 0;
    std::u16string pn;
    std::u16string tn;

    // key:tn , value:portal
    static inline std::unordered_map<std::u16string, SDL_FPoint> pors;
    static inline std::unordered_map<int, std::unordered_map<std::u16string, SDL_FPoint>> pors_cache;

    Portal() = default;
};
