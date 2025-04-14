#pragma once

#include <SDL3/SDL.h>
#include "wz/Property.hpp"
#include <optional>

struct Sprite
{
    struct Wrap
    {
        SDL_Texture *texture = nullptr;
        int delay = 0;
        int a0 = 255;
        int a1 = 255;
        SDL_Point origin = {0, 0};
        wz::Node *n = nullptr;

        static Wrap *load(wz::Node *node, int alpha = 255);
        Wrap(wz::Node *node, int alpha = 255);
    };
    Wrap *sprw = nullptr;
    Sprite(wz::Node *node, int alpha = 255);
    Sprite(Wrap *wrap);
    Sprite() = default;
};
