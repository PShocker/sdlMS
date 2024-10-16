module;

#include <vector>
#include <string>

export module components:effect;

import :animatedsprite;
import :transform;

export struct Effect
{
    struct Info
    {
        Transform *tr;
        AnimatedSprite aspr;
        int delay = 0;
    };

    std::vector<Info> effects;
    Effect() = default;

    static AnimatedSpriteWarp *load(const std::u16string &path);
};
