#include "Npc.h"
#include <SDL3/SDL.h>

SDL_FRect Npc::rect()
{
    auto animated = &a[index];
    auto n_spr = animated->asprw->sprites[animated->anim_index];
    SDL_FRect n_rect;
    if (n_spr->n->get_child(u"lt") && n_spr->n->get_child(u"rb"))
    {
        auto lt = dynamic_cast<wz::Property<wz::WzVec2D> *>(n_spr->n->get_child(u"lt"))->get();
        auto rb = dynamic_cast<wz::Property<wz::WzVec2D> *>(n_spr->n->get_child(u"rb"))->get();
        auto x = lt.x;
        auto y = lt.y;
        auto w = rb.x - lt.x;
        auto h = rb.y - lt.y;
        n_rect = SDL_FRect{(float)x, (float)y, (float)w, (float)h};
    }
    else
    {
        auto x = 0;
        auto y = 0;
        auto w = n_spr->texture->w;
        auto h = n_spr->texture->h;
        n_rect = SDL_FRect{(float)x, (float)y, (float)w, (float)h};
    }
    return n_rect;
}

SDL_FPoint Npc::head()
{
    auto animated = &a[index];
    auto n_spr = animated->asprw->sprites[animated->anim_index];
    SDL_FPoint n_head;
    if (n_spr->n->get_child(u"head"))
    {
        auto v = dynamic_cast<wz::Property<wz::WzVec2D> *>(n_spr->n->get_child(u"head"))->get();
        n_head = SDL_FPoint{(float)v.x, (float)v.y};
    }
    else
    {
        n_head = SDL_FPoint{(float)-n_spr->origin.x, (float)-n_spr->origin.y};
    }
    n_head.x += n_spr->texture->w / 4;
    return n_head;
}
