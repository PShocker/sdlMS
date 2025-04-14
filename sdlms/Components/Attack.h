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

struct Attack
{
    SDL_FRect rect = {0, 0, 0, 0};
    int mobCount = 1;
    int attackCount = 1;
    int damage = 50;

    AnimatedSprite::Wrap *hit = nullptr;
    // 攻击源的坐标
    std::optional<SDL_FPoint> src_point = std::nullopt;
    // 攻击后击中音效
    Sound::Wrap *souw = nullptr;

    // 伤害浮动范围
    float min_damage = 0.8;
    float max_damage = 1.2;

    std::optional<std::function<void(entt::entity, entt::entity, int)>> call_back = std::nullopt;

    Attack() = default;
    Attack(SDL_FPoint &lt, SDL_FPoint &rb, AnimatedSprite::Wrap *hit,
           int mobCount = 1, int attackCount = 1,
           Sound::Wrap *souw = nullptr, int damage = 50);
};