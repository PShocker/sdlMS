#pragma once

#include "Components/Components.h"

struct KeyConfig
{
    static void run();
    static void show();
    static void hide();
    static void over();
    static bool mousein();
    static void click();

    static inline SDL_Texture *backgrnd;

    static inline std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> BtClose;

    static const inline std::map<std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> *, SDL_FRect> position_map = {
        {&BtClose, SDL_FRect{608.0, 7.0, 12.0, 12.0}},
    };

    static void BtClose_func();
    static const inline std::map<std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> *, std::function<void()>> click_map = {
        {&BtClose, BtClose_func},
    };

    static inline bool open = false;
    static inline float x = 0;
    static inline float y = 0;
    static inline int alpha = 255;
};
