#pragma once

#include "AnimatedSprite.h"
#include "Transform.h"

struct Tomb
{
    static inline AnimatedSprite::Wrap *fall;
    static inline AnimatedSprite::Wrap *land;

    Transform f;
    Transform l;

    AnimatedSprite aspr = AnimatedSprite(fall);

    static void init();
    Tomb() = default;
};
