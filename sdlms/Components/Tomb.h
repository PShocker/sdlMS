#pragma once

#include "AnimatedSprite.h"
#include "Transform.h"

struct Tomb
{
    static inline AnimatedSpriteWarp *fall;
    static inline AnimatedSpriteWarp *land;

    Transform f;
    Transform l;

    AnimatedSprite aspr = AnimatedSprite(fall);

    static void init();
    Tomb() = default;
};
