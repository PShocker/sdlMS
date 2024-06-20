#pragma once

#include <SDL2/SDL.h>
#include <vector>
#include <string>

#include "Sprite.h"
#include "wz/Property.hpp"

class AnimatedSprite : public Component
{
private:
    std::vector<Sprite *> sprites;
    int anim_index;
    int anim_time;
    int anim_size;
    bool animate = true; // 是否播放动画,特殊情况,冰冻状态下会暂停动画
    bool z = false;      // ziazag

public:
    AnimatedSprite(wz::Node *node, int alpha = 255);
    ~AnimatedSprite();
    void add_anim_time(const int val) { anim_time += val; }

    auto get_anim_time() { return anim_time; }
    auto get_anim_index() { return anim_index; }
    auto get_anim_size() { return anim_size; }
    auto get_zigzag() { return z; }
    auto get_animate() { return animate; }
    auto get_current_sprite() { return sprites[anim_index]; }
    auto get_anim_delay() { return sprites[anim_index]->delay; }
    auto get_anim_width() { return sprites[anim_index]->get_width(); }
    auto get_anim_height() { return sprites[anim_index]->get_height(); }
    auto &get_sprites() { return sprites; }

    void set_anim_index(const int val) { anim_index = val; }
    void set_anim_time(const int val) { anim_time = val; }
    void set_animate(const bool val) { animate = val; }
};