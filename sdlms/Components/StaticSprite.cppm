module;
#include "wz/Property.hpp"
export module components:staticsprite;

import :sprite;

export struct StaticSprite
{
    Sprite *spr = nullptr;

    StaticSprite(wz::Node *node, int alpha = 255);
    StaticSprite() = default;
};

