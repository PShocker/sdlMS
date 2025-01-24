#include "Trap.h"
#include <SDL3/SDL.h>

Trap::Trap(int damage) : damage(damage) {}

SDL_FRect Trap::rect(AnimatedSprite *aspr)
{
    auto sprw = aspr->aspr->sprites[aspr->anim_index];
    auto rect = sprw->rect.value();
    return rect;
}
