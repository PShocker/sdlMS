module;

#include <unordered_map>
#include <string>
#include <SDL3/SDL.h>

export module components:afterimage;

import :animatedsprite;
import :character;

export struct AfterImage
{
    static inline std::unordered_map<std::u16string, std::unordered_map<uint8_t, std::tuple<uint8_t, AnimatedSprite *, SDL_FPoint, SDL_FPoint>>> swordOS;

    bool animated = false;
    bool animate = false;

    AnimatedSprite *aspr = nullptr;

    static void init();
    AfterImage() = default;
};
