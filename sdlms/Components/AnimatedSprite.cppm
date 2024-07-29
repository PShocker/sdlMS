module;
#include <vector>
#include "wz/Property.hpp"

export module components:animatedsprite;

import :sprite;

export struct AnimatedSprite
{
    std::vector<Sprite *> sprites;

    bool z = false; // ziazag

    AnimatedSprite(wz::Node *node, int alpha = 255);
};

export AnimatedSprite *load_animatedsprite(wz::Node *node, int alpha = 255);


