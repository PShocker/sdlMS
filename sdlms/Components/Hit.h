#pragma once

#include "Sound.h"
#include "AnimatedSprite.h"
#include "entt/entt.hpp"
#include "Core/Window.h"

struct HitWarp
{
    int damage = 0;
    int count = 0;
    SoundWarp *souw = nullptr;

    std::optional<SDL_FPoint> src_point;

    std::optional<SDL_FPoint> hit_point;

    float range = 1;
    AnimatedSpriteWarp *asprw = nullptr;
    entt::entity owner = entt::null;

    int real_damage();
};

struct Hit
{
    std::vector<HitWarp> hits;
};