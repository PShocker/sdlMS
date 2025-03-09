#pragma once

#include "Sound.h"
#include "AnimatedSprite.h"
#include "entt/entt.hpp"

struct HitWarp
{
    int damage = 0;
    int count = 0;
    SoundWarp *souw = nullptr;

    float x;
    float y;

    std::optional<SDL_FPoint> p;

    float range = 1;
    AnimatedSpriteWarp *asprw = nullptr;
    entt::entity owner = entt::null;

    int real_damage();
};

struct Hit
{
    std::vector<HitWarp> hits;
};