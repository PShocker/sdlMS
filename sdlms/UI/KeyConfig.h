#pragma once

#include "Components/Components.h"

struct KeyConfig
{
    static void init();
    static void show();
    static void hide();

    static inline Sprite backgrnd;

    static inline bool open = false;
    static inline bool keyborard = true;
    static inline float x = 0;
    static inline float y = 0;
    static inline int alpha = 255;
};
