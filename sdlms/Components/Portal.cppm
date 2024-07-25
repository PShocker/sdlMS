module;

#include <string>
#include <variant>
#include <SDL3/SDL.h>

export module components:portal;

import :transform;
import :animated;

export struct Portal
{
    int tm = 0;
    int pt = 0;
    std::u16string pn;
    // std::u16string tn;
    std::variant<std::u16string, SDL_FPoint> tn;

    // 三段式传送门
    std::vector<Animated *> a;

    Portal() = default;
};
