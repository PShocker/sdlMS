module;

#include <string>
#include <variant>
#include <SDL3/SDL.h>

export module components:portal;

import :transform;

export struct Portal
{
    int tm = 0;
    int pt = 0;
    std::u16string pn;
    // std::u16string tn;
    std::variant<std::u16string, SDL_FPoint> tn;

    Portal() = default;
};
