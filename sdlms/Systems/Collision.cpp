module;

#include <SDL3/SDL.h>
#include "entt/entt.hpp"

module systems;

import components;
import core;
import commons;

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

bool collision(AttackWarp *atkw, Mob *mob, Transform *m_tr)
{
    auto p_tr = World::registry->try_get<Transform>(Player::ent);

    return collision(mob, m_tr, atkw->rect, p_tr);
}

bool collision(AttackWarp *atkw, Npc *npc, Transform *n_tr)
{
    auto p_tr = World::registry->try_get<Transform>(Player::ent);

    return collision(npc, n_tr, atkw->rect, p_tr);
}

bool collision(AttackWarp *atkw, Character *cha, Transform *c_tr)
{
    auto p_tr = World::registry->try_get<Transform>(Player::ent);

    return collision(cha, c_tr, atkw->rect, p_tr);
}

bool collision(Mob *mob, Transform *m_tr)
{
    auto p_tr = World::registry->try_get<Transform>(Player::ent);
    auto p_cha = World::registry->try_get<Character>(Player::ent);

    return collision(mob, m_tr, p_cha->r, p_tr);
}

bool collision(Trap *trap, AnimatedSprite *aspr, Transform *t_tr)
{
    auto p_tr = World::registry->try_get<Transform>(Player::ent);
    auto p_cha = World::registry->try_get<Character>(Player::ent);

    return collision(trap->rect(aspr), t_tr, p_cha->r, p_tr);
}

bool collision(Mob *mob, Transform *m_tr, SDL_FRect n_rect, Transform *n_tr)
{
    return collision(mob->rect(), m_tr, n_rect, n_tr);
}

bool collision(Npc *npc, Transform *m_tr, SDL_FRect n_rect, Transform *n_tr)
{
    return collision(npc->rect(), m_tr, n_rect, n_tr);
}

bool collision(Character *cha, Transform *c_tr, SDL_FRect n_rect, Transform *n_tr)
{
    return collision(cha->r, c_tr, n_rect, n_tr);
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

    n_rect.x += n_tr->position.x;
    n_rect.y += n_tr->position.y;
    if (n_tr->flip == 1)
    {
        n_rect.x += 2 * (n_tr->position.x - n_rect.x) - n_rect.w;
    }
    return RectIntersectTriangle(n_rect, t);
}

bool collision(Mob *mob, Transform *m_tr, Triangle t, Transform *n_tr)
{
    return collision(t, n_tr, mob->rect(), m_tr);
}

bool collision(Character *cha, Transform *c_tr, Triangle t, Transform *n_tr)
{
    return collision(t, n_tr, cha->r, c_tr);
}

SDL_FRect real_rect(SDL_FRect rect, Transform *tr)
{
    rect.x += tr->position.x;
    rect.y += tr->position.y;
    if (tr->flip == 1)
    {
        rect.x += 2 * (tr->position.x - rect.x) - rect.w;
    }
    return rect;
}
