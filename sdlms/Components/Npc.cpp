#include "Npc.h"

#include <SDL3/SDL.h>


SDL_FRect Npc::rect()
{
    auto animated = a[index];
    auto n_spr = animated->aspr->sprites[animated->anim_index];
    return n_spr->rect.value();
}

SDL_FPoint Npc::head()
{
    auto animated = a[index];
    auto n_spr = animated->aspr->sprites[animated->anim_index];
    return n_spr->head.value();
}
