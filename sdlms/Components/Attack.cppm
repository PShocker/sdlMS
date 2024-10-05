module;

#include <SDL3/SDL.h>
#include <vector>

export module components:attack;

import :animatedsprite;

export struct AttackWarp
{
    SDL_FRect rect;
    int mobCount = 1;
    int attackCount;
    int damage;

    AnimatedSpriteWarp *hit;

    bool finish = false;

    // 攻击源的坐标
    SDL_FPoint *p;
};

export struct Attack
{
    std::vector<AttackWarp> atks;
    Attack() = default;
};