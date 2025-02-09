#pragma once

#include "Sound.h"
#include "entt/entt.hpp"
#include "Core/Player.h"

struct Hit
{
    int damage = 0;
    int count = 0;

    SoundWarp *souw = nullptr;

    float x;
    float y;

    entt::entity owner = entt::null;
};