#pragma once

#include "AnimatedSprite.h"
#include "Sound.h"

#include <unordered_map>
#include <string>
#include <SDL3/SDL.h>

struct AfterImage
{
    struct Wrap
    {
        uint8_t index;
        AnimatedSprite::Wrap *asprw;
        SDL_FPoint lt;
        SDL_FPoint rb;
    };

    static inline std::unordered_map<std::u16string, std::unordered_map<std::u16string, std::unordered_map<uint8_t, Wrap>>> afterimages;

    static inline std::unordered_map<std::u16string, AnimatedSprite::Wrap *> hits;
    static inline std::unordered_map<std::u16string, std::vector<Sound::Wrap *>> sounds;

    bool animated = false;
    bool animate = false;

    AnimatedSprite aspr;
    Wrap wrap;

    bool hit = false;

    static void load(const std::u16string &type, const std::u16string &sfx, const std::u16string &level);
    AfterImage() = default;
};
