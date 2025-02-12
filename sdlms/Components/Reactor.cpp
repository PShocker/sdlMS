#include "Reactor.h"

SDL_FRect Reactor::rect()
{
    auto init = &a[index].init;
    auto r_spr = init->aspr->sprites[init->anim_index];
    return r_spr->rect.value();
}

SDL_FPoint Reactor::head()
{
    auto init = &a[index].init;
    auto r_spr = init->aspr->sprites[init->anim_index];
    auto head = r_spr->head.value();
    head.x += r_spr->rect.value().w / 4;
    return head;
}
