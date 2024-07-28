module;

#include <SDL3/SDL.h>
#include <optional>

export module components:attack;

import :playerskill;
import :animatedsprite;

export struct Attack
{
    SDL_FRect rect;
    int mobCount;
    int attackCount;
    int damage;

    AnimatedSprite *hit;

    std::optional<int> x = std::nullopt;
    std::optional<int> y = std::nullopt;

    Attack(PlayerSkill *pski);
};
