#pragma once

#include <SDL3/SDL.h>
#include <any>
#include "wz/Property.hpp"
#include <optional>

struct SpriteWarp
{
    SDL_Texture *texture = nullptr;
    int delay = 0;
    int a0 = 255;
    int a1 = 255;
    SDL_Point origin = {0, 0};
    wz::Node *n = nullptr;

    static SpriteWarp *load(wz::Node *node, int alpha = 255, bool caches = true);
    static void clean_up();

    SpriteWarp(wz::Node *node, int alpha = 255);
    SpriteWarp() = default;
};

struct Sprite
{
    SpriteWarp *sprw = nullptr;
    Sprite(wz::Node *node, int alpha = 255);
    Sprite(SpriteWarp *sprw);
    Sprite() = default;
};
