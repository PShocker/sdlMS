module;

#include "wz/Property.hpp"

module components;

import :animatedsprite;

Animated::Animated(AnimatedSprite *aspr) : aspr(aspr)
{
    anim_size = aspr->sprites.size();
}

Animated::Animated(wz::Node *node, int alpha)
{
    aspr = load_animatedsprite(node, alpha);
    anim_size = aspr->sprites.size();
    anim_index = 0;
    anim_time = 0;
}
