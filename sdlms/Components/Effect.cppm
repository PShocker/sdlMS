module;

#include <vector>
#include <string>

export module components:effect;

import :animatedsprite;
import :transform;
import core;

export struct Effect
{
    struct Info
    {
        Transform *tr;
        AnimatedSprite aspr;
        int delay = Window::dt_now;
    };

    std::vector<Info> effects;
    Effect() = default;

    static AnimatedSpriteWarp *load(const std::u16string &path);
};
