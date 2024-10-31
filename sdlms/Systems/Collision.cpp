module;

#include <SDL3/SDL.h>
#include "entt/entt.hpp"

module systems;

import components;
import core;

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
    auto sprw = aspr->aspr->sprites[aspr->anim_index];

    auto p_tr = World::registry->try_get<Transform>(Player::ent);
    auto p_cha = World::registry->try_get<Character>(Player::ent);

    return collision(sprw->rect.value(), t_tr, p_cha->r, p_tr);
}

bool collision(Mob *mob, Transform *m_tr, SDL_FRect n_rect, Transform *n_tr)
{
    auto animated = mob->a[mob->index];
    auto m_spr = animated->aspr->sprites[animated->anim_index];
    return collision(m_spr->rect.value(), m_tr, n_rect, n_tr);
}

bool collision(Npc *npc, Transform *m_tr, SDL_FRect n_rect, Transform *n_tr)
{
    auto animated = npc->a[npc->index];
    auto n_spr = animated->aspr->sprites[animated->anim_index];
    return collision(n_spr->rect.value(), m_tr, n_rect, n_tr);
}

bool collision(Character *cha, Transform *m_tr, SDL_FRect n_rect, Transform *n_tr)
{
    return collision(cha->r, m_tr, n_rect, n_tr);
}
