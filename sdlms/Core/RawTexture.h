#pragma once
#include "wz/Property.hpp"
#include <SDL3/SDL.h>

struct RawTexture
{
    static std::vector<u8> *load(wz::Property<wz::WzCanvas> *node);
};