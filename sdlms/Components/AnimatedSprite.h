#pragma once

#include <SDL2/SDL.h>
#include <vector>
#include <string>

#include "Sprite.h"
#include "wz/Property.hpp"

class AnimatedSprite : public Component
{
public:
    static AnimatedSprite *load_animated_sprite(wz::Node *node);
    static inline std::map<wz::Node *, AnimatedSprite *> animatedsprite_map;
    bool update(int delta_time);

    void add_anim_time(const int val) { anim_time += val; }
    void set_anim_time(const int val) { anim_time = val; }
    constexpr auto get_anim_delay() { return sprites[anim_index]->delay; }
    constexpr auto get_anim_time() { return anim_time; }
    void advance_anim();

private:
    AnimatedSprite(wz::Node *node);

public:
    std::vector<Sprite *> sprites;
    int anim_index;
    int anim_time;
    int anim_size;
    bool z = false; // ziazag
};