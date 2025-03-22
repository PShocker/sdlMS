#pragma once

#include "AnimatedSprite.h"

#include <unordered_map>
#include <string>
#include <SDL3/SDL.h>

struct Npc
{
    std::u16string id;

    std::unordered_map<std::u16string, AnimatedSprite> a;

    std::u16string index;
};
