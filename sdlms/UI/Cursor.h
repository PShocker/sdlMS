#pragma once

#include <string>
#include <unordered_map>
#include "Components/Components.h"

struct Cursor
{
    static inline std::unordered_map<std::u16string, AnimatedSprite *> cursor;

    static void init();
};
