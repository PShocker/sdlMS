#pragma once

#include "AnimatedSprite.h"

#include <variant>

struct BackGround
{
    std::variant<Sprite, AnimatedSprite> spr;
    // 平铺间隔
    int cx = 0;
    int cy = 0;
    // 是否平铺
    bool htile = false;
    bool vtile = false;

    // 平移属性
    int rx = 0;
    int ry = 0;
    float offset_x = 0.0f;
    float offset_y = 0.0f;
    bool hspeed = false;
    bool vspeed = false;

    BackGround() = default;
};