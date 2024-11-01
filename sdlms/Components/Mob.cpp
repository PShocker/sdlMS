module;

#include <SDL3/SDL.h>

module components;

SDL_FRect Mob::rect()
{
    auto animated = a[index];
    auto m_spr = animated->aspr->sprites[animated->anim_index];
    return m_spr->rect.value();
}

SDL_FPoint Mob::head()
{
    auto animated = a[index];
    auto m_spr = animated->aspr->sprites[animated->anim_index];
    return m_spr->head.value();
}
