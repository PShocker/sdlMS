#pragma once

#include <SDL2/SDL.h>
#include <vector>
#include <string>

#include "Sprite.hpp"
#include "wz/Property.hpp"

class AnimatedSprite : public Component
{
public:
    AnimatedSprite(wz::Node *node, int x = 0, int y = 0, int flip = SDL_FLIP_NONE);

    bool update(int elapsedTime);

public:
    std::vector<Sprite *> sprites;
    int frame_index;
    int frame_time;
    int frame_size;
    bool z;
};