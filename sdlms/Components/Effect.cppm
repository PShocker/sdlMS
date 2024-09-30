module;

#include <vector>
#include <tuple>

export module components:effect;

import :animatedsprite;
import :transform;

export struct Effect
{
    std::vector<std::pair<Transform *, AnimatedSprite>> effects;
    Effect() = default;

    static AnimatedSpriteWarp *load(const std::u16string &path);
};

