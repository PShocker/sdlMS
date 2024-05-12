#pragma once

#include <SDL2/SDL.h>
#include <vector>
#include <string>

#include "Sprite.h"
#include "wz/Property.hpp"

class AnimatedSprite : public Component
{
public:
    AnimatedSprite(wz::Node *node);
    ~AnimatedSprite();
    void add_anim_time(const int val) { anim_time += val; }
    void set_anim_time(const int val) { anim_time = val; }
    constexpr auto get_anim_delay() { return sprites[anim_index]->delay; }
    constexpr auto get_anim_time() { return anim_time; }
    constexpr auto get_current_sprite() { return sprites[anim_index]; }
    auto get_anim_width() { return sprites[anim_index]->get_width(); }
    auto get_anim_height() { return sprites[anim_index]->get_height(); }
    void advance_anim();
    void set_anim_index(const int val) { anim_index = val; }

public:
    std::vector<Sprite *> sprites;
    int anim_index;
    int anim_time;
    int anim_size;
    bool z = false; // ziazag
};