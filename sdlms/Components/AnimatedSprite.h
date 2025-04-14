#pragma once

#include "Sprite.h"
#include <vector>
#include <unordered_set>
#include "wz/Property.hpp"

struct AnimatedSprite
{
    struct Wrap
    {
        std::vector<Sprite::Wrap *> sprites;
        bool z = false; // ziazag
        static Wrap *load(wz::Node *node, int alpha = 255);
        Wrap(wz::Node *node, int alpha = 255);
    };

    Wrap *asprw = nullptr;

    int anim_index = 0;
    int anim_time = 0;
    int anim_size = 0;
    bool animate = true; // 是否播放
    int alpha = 255;     // 当前帧所对应的透明度
    char anim_step = 1;

    AnimatedSprite(wz::Node *node, int alpha = 255);
    AnimatedSprite(AnimatedSprite::Wrap *asprw);
    AnimatedSprite() = default;
};