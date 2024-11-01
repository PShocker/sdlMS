module;

#include <SDL3/SDL.h>

module components;

import :animatedsprite;

Trap::Trap(int damage) : damage(damage) {}

SDL_FRect Trap::rect(AnimatedSprite *aspr)
{
    auto sprw = aspr->aspr->sprites[aspr->anim_index];
    auto rect = sprw->rect.value();
    return rect;
}
