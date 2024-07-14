#pragma once

#include <SDL3/SDL.h>
#include <vector>
#include <string>

#include "Sprite.h"
#include "wz/Property.hpp"

struct AnimatedSprite
{
    std::vector<Sprite *> sprites;
    int anim_index=0;
    int anim_time=0;
    int anim_size=0;
    bool animate = true; // 是否播放动画,特殊情况,冰冻状态下会暂停动画
    bool z = false;      // ziazag

    AnimatedSprite(wz::Node *node, int alpha = 255);
};