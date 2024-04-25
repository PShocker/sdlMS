#pragma once

#include <SDL2/SDL.h>
#include <vector>
#include <map>
#include <string>
#include "Core/Point.h"
#include "wz/Property.hpp"
#include "Component.h"

class Sprite : public Component
{
public:
    Sprite(wz::Node *node, Point<int32_t> p, int flip = SDL_FLIP_NONE);
    Sprite(wz::Node *node, int x = 0, int y = 0, int flip = SDL_FLIP_NONE);
    Sprite(SDL_Texture *texture, SDL_FRect rect, int flip = SDL_FLIP_NONE);
    Sprite(){};

public:
    SDL_Texture *texture;
    SDL_FRect rect;
    std::vector<uint8_t> raw_data;
    unsigned int format;
    int flip;
    int delay;
    int a0;
    int a1;

    std::u16string url;

    static inline std::map<std::u16string, SDL_Texture *> texture_map;
};