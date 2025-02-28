#include "Reactor.h"

SDL_FRect Reactor::rect()
{
    auto init = &a[index].init;
    auto r_spr = init->asprw->sprites[init->anim_index];
    SDL_FRect r_rect;
    if (r_spr->n->get_child(u"lt") && r_spr->n->get_child(u"rb"))
    {
        auto lt = dynamic_cast<wz::Property<wz::WzVec2D> *>(r_spr->n->get_child(u"lt"))->get();
        auto rb = dynamic_cast<wz::Property<wz::WzVec2D> *>(r_spr->n->get_child(u"rb"))->get();
        auto x = lt.x;
        auto y = lt.y;
        auto w = rb.x - lt.x;
        auto h = rb.y - lt.y;
        r_rect = SDL_FRect{(float)x, (float)y, (float)w, (float)h};
    }
    else
    {
        auto x = 0;
        auto y = 0;
        auto w = r_spr->texture->w;
        auto h = r_spr->texture->h;
        r_rect = SDL_FRect{(float)x, (float)y, (float)w, (float)h};
    }
    return r_rect;
}

SDL_FPoint Reactor::head()
{
    auto init = &a[index].init;
    auto r_spr = init->asprw->sprites[init->anim_index];
    SDL_FPoint r_head;
    if (r_spr->n->get_child(u"head"))
    {
        auto v = dynamic_cast<wz::Property<wz::WzVec2D> *>(r_spr->n->get_child(u"head"))->get();
        r_head = SDL_FPoint{(float)v.x, (float)v.y};
    }
    else
    {
        r_head = SDL_FPoint{(float)-r_spr->origin.x, (float)-r_spr->origin.y};
    }
    r_head.x += r_spr->texture->w / 4;
    return r_head;
}
