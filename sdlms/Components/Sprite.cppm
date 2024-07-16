module;

#include <SDL3/SDL.h>
#include <vector>
#include <map>
#include <string>
#include <any>
#include "wz/Property.hpp"

export module components:sprite;

export struct Sprite
{
    SDL_Texture *texture = nullptr;
    int width = 0;
    int height = 0;
    int delay = 0;
    int a0 = 255;
    int a1 = 255;
    SDL_Point origin = {0, 0};
    std::any z = 0;

    Sprite(SDL_Texture *texture, int w, int h);
    Sprite(wz::Node *node, int width, int height, uint8_t type);
    Sprite(wz::Node *node, int alpha = 255);
};