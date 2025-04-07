#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "Components/Components.h"

struct Cursor
{
    static void init();
    static void load();
    static void run();
    static void action(const std::u16string &index);

    static inline std::unordered_map<std::u16string, AnimatedSprite> a;
    static inline std::u16string index = u"0";

    static inline bool left_mouse_press;
    static inline bool right_mouse_press;
    static inline bool drag;

    static inline float x = 0;
    static inline float y = 0;
};
