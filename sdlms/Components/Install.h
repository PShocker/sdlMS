#pragma once

#include "AnimatedSprite.h"
#include <string>

struct Install
{
    std::u16string id;
    AnimatedSprite aspr;

    Install(const std::u16string &id);
};