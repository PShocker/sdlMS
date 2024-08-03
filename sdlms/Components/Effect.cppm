module;

#include <vector>

export module components:effect;

import :animatedsprite;

export struct Effect
{
    std::vector<AnimatedSprite> effects;
    Effect() = default;
};