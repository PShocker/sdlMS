#pragma once
#include "wz/Property.hpp"
#include <SDL3/SDL.h>

struct Texture
{
    static SDL_Texture *load(wz::Property<wz::WzCanvas> *node);
};