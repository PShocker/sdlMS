module;

#include <SDL3/SDL.h>

export module components:attack;

import :animatedsprite;

export struct Attack
{
    SDL_FRect rect;
    int mobCount;
    int attackCount;
    int damage;

    AnimatedSpriteWarp *hit;

    bool finish = false;

    // 攻击源的坐标
    SDL_FPoint *p;
};
