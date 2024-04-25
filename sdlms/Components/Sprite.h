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
    Sprite(wz::Node *node);
    Sprite(SDL_Texture *texture);
    Sprite(){};

public:
    SDL_Texture *texture;
    int width;
    int height;

    int delay;
    int a0;
    int a1;
    Point<int> origin;

    std::u16string url;

    static inline std::map<std::u16string, SDL_Texture *> texture_map;
};