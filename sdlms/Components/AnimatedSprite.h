#pragma once

#include "Sprite.h"
#include <vector>
#include <unordered_set>
#include "wz/Property.hpp"

struct AnimatedSpriteWarp
{
    std::vector<SpriteWarp *> sprites;

    bool z = false; // ziazag

    AnimatedSpriteWarp(wz::Node *node, int alpha = 255, bool caches = true);

    static AnimatedSpriteWarp *load(wz::Node *node, int alpha = 255, bool caches = true);
    static void clean_up();
};

struct AnimatedSprite
{
    int anim_index = 0;
    int anim_time = 0;
    int anim_size = 0;
    bool animate = true; // 是否播放
    int alpha = 255;     // 当前帧所对应的透明度

    AnimatedSpriteWarp *aspr = nullptr;

    char anim_step = 1;

    AnimatedSprite(wz::Node *node, int alpha = 255);
    AnimatedSprite(AnimatedSpriteWarp *aspr);
    AnimatedSprite() = default;
};