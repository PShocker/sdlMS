module;

export module components:hit;

import :sound;

export struct Hit
{
    int damage = 0;
    int count = 1;

    SoundWarp *souw = nullptr;

    float x;
    float y;
};