#include "Mob.h"
#include <SDL3/SDL.h>

SDL_FRect Mob::rect()
{
    auto animated = &a[index];
    auto m_spr = animated->asprw->sprites[animated->anim_index];
    SDL_FRect m_rect;
    if (m_spr->n->get_child(u"lt") && m_spr->n->get_child(u"rb"))
    {
        auto lt = dynamic_cast<wz::Property<wz::WzVec2D> *>(m_spr->n->get_child(u"lt"))->get();
        auto rb = dynamic_cast<wz::Property<wz::WzVec2D> *>(m_spr->n->get_child(u"rb"))->get();
        auto x = lt.x;
        auto y = lt.y;
        auto w = rb.x - lt.x;
        auto h = rb.y - lt.y;
        m_rect = SDL_FRect{(float)x, (float)y, (float)w, (float)h};
    }
    else
    {
        auto x = 0;
        auto y = 0;
        auto w = m_spr->texture->w;
        auto h = m_spr->texture->h;
        m_rect = SDL_FRect{(float)x, (float)y, (float)w, (float)h};
    }
    return m_rect;
}

SDL_FPoint Mob::head(int flip)
{
    auto animated = &a[index];
    auto m_spr = animated->asprw->sprites[animated->anim_index];
    SDL_FPoint m_head;
    if (m_spr->n->get_child(u"head"))
    {
        auto v = dynamic_cast<wz::Property<wz::WzVec2D> *>(m_spr->n->get_child(u"head"))->get();
        if (flip == 1)
        {
            m_head = SDL_FPoint{(float)-v.x, (float)v.y};
        }
        else
        {
            m_head = SDL_FPoint{(float)v.x, (float)v.y};
        }
    }
    else
    {
        m_head = SDL_FPoint{(float)-m_spr->origin.x, (float)-m_spr->origin.y};
    }
    return m_head;
}
