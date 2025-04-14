#pragma once

#include "Components/Components.h"

struct UINotice
{
    static void show();
    static void hide();
    static void over();
    static bool mousein();
    static void click();

    static inline SDL_Texture *backgrnd;

    static inline std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> BtOK2;

    static const inline std::map<std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> *, SDL_FRect> position_map = {
        {&BtOK2, SDL_FRect{120, 117, 47.0, 19.0}},
    };

    static void BtOK2_func();
    static const inline std::map<std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> *, std::function<void()>> click_map = {
        {&BtOK2, BtOK2_func},
    };

    static inline bool open = false;
    static inline float x = 0;
    static inline float y = 0;
    static inline int alpha = 255;
};
