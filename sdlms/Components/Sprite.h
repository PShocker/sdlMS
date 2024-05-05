#pragma once

#include <SDL2/SDL.h>
#include <vector>
#include <map>
#include <string>
#include "wz/Property.hpp"
#include "Component.h"

class Sprite : public Component
{
public:
    Sprite(SDL_Texture *texture, int width, int height);
    static Sprite *load_sprite(wz::Node *node);
    static inline std::map<wz::Node *, Sprite *> sprite_map;

private:
    Sprite(wz::Node *node);
    Sprite(){};

public:
    const auto get_width() { return width; }
    const auto get_height() { return height; }
    const auto get_origin() { return origin; }
    SDL_Texture *get_texture() { return texture; }

public:
    SDL_Texture *texture;
    int width;
    int height;

    int delay = 0;
    int a0 = 255;
    int a1 = 255;
    SDL_Point origin = {0, 0};

    int z;
};