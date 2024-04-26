#pragma once

#include <SDL2/SDL.h>
#include <vector>
#include <string>

#include "Sprite.h"
#include "wz/Property.hpp"

class AnimatedSprite : public Component
{
public:
    static AnimatedSprite *load_animatedsprite(wz::Node *node);
    static inline std::map<wz::Node *, AnimatedSprite *> animatedsprite_map;

private:
    AnimatedSprite(wz::Node *node);

    bool update(int elapsedTime);

public:
    std::vector<Sprite *> sprites;
    int frame_index;
    int frame_time;
    int frame_size;
    bool z; //ziazag
};