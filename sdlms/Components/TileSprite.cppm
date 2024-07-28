module;
#include "wz/Property.hpp"
export module components:tilesprite;

import :sprite;

export struct TileSprite
{
    Sprite *spr = nullptr;

    TileSprite(wz::Node *node, int alpha = 255);
    TileSprite() = default;
};

