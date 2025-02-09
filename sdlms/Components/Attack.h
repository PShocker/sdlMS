#pragma once

#include "AnimatedSprite.h"
#include "Sound.h"
#include "Transform.h"
#include "WeaponInfo.h"

#include <SDL3/SDL.h>
#include <vector>

struct AttackWarp
{
    SDL_FRect rect = {0, 0, 0, 0};
    int mobCount = 1;
    int attackCount = 1;
    int damage = 1;

    AnimatedSpriteWarp *hit = nullptr;

    // 攻击源的坐标
    SDL_FPoint *p = nullptr;

    // 攻击后击中音效
    SoundWarp *souw = nullptr;

    AttackWarp() = default;
    AttackWarp(SDL_FPoint &lt, SDL_FPoint &rb, AnimatedSpriteWarp *hit,
               int mobCount = 1, int attackCount = 1, SoundWarp *souw = nullptr);
};

struct Attack
{
    std::vector<AttackWarp> atks;
    Attack() = default;
};
