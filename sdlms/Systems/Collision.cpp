#include "Collision.h"
#include "Rect.h"
#include "Core/Core.h"
#include <SDL3/SDL.h>
#include "entt/entt.hpp"

bool collision(SDL_FRect m_rect, Transform *m_tr, SDL_FRect n_rect, Transform *n_tr)
{
    m_rect = real_rect(m_rect, m_tr);
    n_rect = real_rect(n_rect, n_tr);
    SDL_FRect intersection;
    if (SDL_GetRectIntersectionFloat(&m_rect, &n_rect, &intersection))
    {
        return true;
    }
    return false;
}

bool collision(Triangle t, Transform *t_tr, SDL_FRect n_rect, Transform *n_tr)
{
    if (t_tr->flip)
    {
        t.vertex1.x = t_tr->position.x - t.vertex1.x;
        t.vertex1.y = t_tr->position.y - t.vertex1.y;
        t.vertex2.x = t_tr->position.x - t.vertex2.x;
        t.vertex2.y = t_tr->position.y - t.vertex2.y;
        t.vertex3.x = t_tr->position.x - t.vertex3.x;
        t.vertex3.y = t_tr->position.y - t.vertex3.y;
    }
    else
    {
        t.vertex1.x += t_tr->position.x;
        t.vertex1.y += t_tr->position.y;
        t.vertex2.x += t_tr->position.x;
        t.vertex2.y += t_tr->position.y;
        t.vertex3.x += t_tr->position.x;
        t.vertex3.y += t_tr->position.y;
    }
    auto rect = real_rect(n_rect, n_tr);
    return RectIntersectTriangle(rect, t);
}