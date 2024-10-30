module;

#include <SDL3/SDL.h>

module systems;

import components;


bool collision(SDL_FRect m_rect, Transform *m_tr, SDL_FRect n_rect, Transform *n_tr)
{
    m_rect.x += m_tr->position.x;
    m_rect.y += m_tr->position.y;
    if (m_tr->flip == 1)
    {
        m_rect.x += 2 * (m_tr->position.x - m_rect.x) - m_rect.w;
    }
    n_rect.x += n_tr->position.x;
    n_rect.y += n_tr->position.y;
    if (n_tr->flip == 1)
    {
        n_rect.x += 2 * (n_tr->position.x - n_rect.x) - n_rect.w;
    }
    SDL_FRect intersection;
    if (SDL_GetRectIntersectionFloat(&m_rect, &n_rect, &intersection))
    {
        return true;
    }
    return false;
}