#pragma once
#include "wz/Property.hpp"
#include "AnimatedSprite.h"
#include "Transform.h"
#include <vector>

// 区域特效，通常是弓弩手的特效，例如箭雨
struct AreaEffect
{
    struct Wrap
    {
        AnimatedSprite aspr;
        Transform tr;
    };

    std::vector<Wrap> effects;

    std::vector<AnimatedSprite> asprs;

    int x;
    int y;

    int fall;
    int start;

    int interval;
    int count;

    unsigned int duration;

    AreaEffect() = default;
};