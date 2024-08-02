module;
#include <vector>
#include "wz/Property.hpp"

export module components:animatedsprite;

import :sprite;

export struct AnimatedSpriteWarp
{
    std::vector<SpriteWarp *> sprites;

    bool z = false; // ziazag

    AnimatedSpriteWarp(wz::Node *node, int alpha = 255);

    static AnimatedSpriteWarp *load(wz::Node *node, int alpha = 255);
};

export struct AnimatedSprite
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