#pragma once

#include "AnimatedSprite.h"
#include "Sound.h"
#include "Skill.h"
#include "Mob.h"
#include "AfterImage.h"
#include "Transform.h"
#include "Trap.h"

#include <SDL3/SDL.h>
#include <vector>

struct AttackWarp
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
    AttackWarp(Mob *mob, Transform *tr);
    AttackWarp(Trap *trap, Transform *tr);
};

struct Attack
{
    std::vector<AttackWarp> atks;
    Attack() = default;
};
