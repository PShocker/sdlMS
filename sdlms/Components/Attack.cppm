module;

#include <SDL3/SDL.h>
#include <vector>

export module components:attack;

import :animatedsprite;
import :sound;
import :skill;
import :afterimage;

export struct AttackWarp
{
    SDL_FRect rect;
    int mobCount = 1;
    int attackCount = 1;
    int damage = 1;

    AnimatedSpriteWarp *hit;

    // 攻击源的坐标
    SDL_FPoint *p;

    // 攻击后击中音效
    SoundWarp *souw = nullptr;

    AttackWarp() = default;
    AttackWarp(Skill *ski, int level = 1);
    AttackWarp(AfterImage *aft);
};

export struct Attack
{
    std::vector<AttackWarp> atks;
    Attack() = default;
};
