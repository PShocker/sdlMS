#pragma once

#include "AnimatedSprite.h"
#include "Sound.h"
#include "Transform.h"
#include "WeaponInfo.h"
#include "entt/entt.hpp"
#include <optional>
#include <functional>
#include <SDL3/SDL.h>
#include <vector>

struct AttackWarp
{
    SDL_FRect rect = {0, 0, 0, 0};
    int mobCount = 1;
    int attackCount = 1;
    int damage = 50;

    AnimatedSpriteWarp *hit = nullptr;

    // 攻击源的坐标
    optional<SDL_FPoint> p = std::nullopt;

    // 攻击后击中音效
    SoundWarp *souw = nullptr;

    std::optional<std::function<void(entt::entity)>> call_back = std::nullopt;

    AttackWarp() = default;
    AttackWarp(SDL_FPoint &lt, SDL_FPoint &rb, AnimatedSpriteWarp *hit,
               int mobCount = 1, int attackCount = 1,
               SoundWarp *souw = nullptr, int damage = 50);
};

struct Attack
{
    std::vector<AttackWarp> atks;
    Attack() = default;
};
