#pragma once

#include "Sound.h"

struct Hit
{
    int damage = 0;
    int count = 0;

    SoundWarp *souw = nullptr;

    float x;
    float y;
};