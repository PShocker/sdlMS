module;

#include "entt/entt.hpp"
#include <SDL3/SDL.h>

module systems;

import core;
import components;

void attack_run()
{
    if (auto atk = World::registry->try_get<Attack>(Player::ent))
    {
        atk->damage = std::rand() % 99 + 1;
        player_attack(atk);
        World::registry->remove<Attack>(Player::ent);
    }
    if (player_invincible_cooldown <= 0)
    {
        auto view = World::registry->view<Mob>();
        for (auto &ent : view)
        {
            auto mob = &view.get<Mob>(ent);
            if (!(mob->state == Mob::State::DIE || mob->state == Mob::State::REMOVE))
            {
                if (mob_collision(mob, World::registry->try_get<Transform>(ent)))
                {
                    return;
                }
            }
        }
    }
}

void player_attack(Attack *atk)
{
    auto view = World::registry->view<Damage>();
    for (auto ent : view)
    {
        if (ent != Player::ent)
        {
            if (auto mob = World::registry->try_get<Mob>(ent))
            {
                if (!(mob->state == Mob::State::DIE || mob->state == Mob::State::REMOVE))
                {
                    attack_mob(atk, mob, &ent);
                }
            }
            else if (auto npc = World::registry->try_get<Npc>(ent))
            {
                attack_npc(atk, npc, &ent);
            }
            else if (auto cha = World::registry->try_get<Character>(ent))
            {
                attack_cha(atk, cha, &ent);
            }
        }
    }
}

bool mob_collision(Mob *mob, Transform *m_tr)
{
    auto animated = mob->a[mob->index];
    auto sprw = animated->aspr->sprites[animated->anim_index];
    if (collision(sprw, m_tr))
    {
        Attack atk;
        atk.damage = 100;
        atk.hit = nullptr;
        atk.p = &m_tr->position;
        hit_effect(&atk, nullptr, &Player::ent, 1);
        return true;
    }
    return false;
}

void attack_mob(Attack *atk, Mob *mob, entt::entity *ent)
{
    auto animated = mob->a[mob->index];
    auto m_spr = animated->aspr->sprites[animated->anim_index];

    auto p_tr = World::registry->try_get<Transform>(Player::ent);
    auto m_tr = World::registry->try_get<Transform>(*ent);

    if (collision(m_spr, m_tr, atk, p_tr))
    {
        hit_effect(atk, &m_spr->head.value(), ent, 0);
    }
}

void attack_npc(Attack *atk, Npc *npc, entt::entity *ent)
{
    atk->damage = 0;

    auto animated = npc->a[npc->index];
    auto n_spr = animated->aspr->sprites[animated->anim_index];

    auto p_tr = World::registry->try_get<Transform>(Player::ent);
    auto n_tr = World::registry->try_get<Transform>(*ent);

    if (collision(n_spr, n_tr, atk, p_tr))
    {
        hit_effect(atk, &n_spr->head.value(), ent, 0);
    }
}

void attack_cha(Attack *atk, Character *cha, entt::entity *ent)
{
    auto p_tr = World::registry->try_get<Transform>(Player::ent);
    auto n_tr = World::registry->try_get<Transform>(*ent);

    if (collision(cha, n_tr, atk, p_tr))
    {
        hit_effect(atk, nullptr, ent, 0);
    }
}

bool collision(SpriteWarp *m_spr, Transform *m_tr, Attack *atk, Transform *p_tr)
{
    auto lt = m_spr->lt;
    auto rb = m_spr->rb;
    SDL_FRect m;
    if (lt.has_value() && rb.has_value())
    {
        m.x = lt.value().x;
        m.y = lt.value().y;
        m.w = rb.value().x - lt.value().x;
        m.h = rb.value().y - lt.value().y;
        m.x += m_tr->position.x;
        m.y += m_tr->position.y;
    }
    else
    {
        m.x = m_tr->position.x;
        m.y = m_tr->position.y;
        m.w = m_spr->width;
        m.h = m_spr->height;
    }
    if (m_tr->flip == 1)
    {
        m.x += 2 * (m_tr->position.x - m.x) - m.w;
    }

    SDL_FRect n = atk->rect;
    n.x += p_tr->position.x;
    n.y += p_tr->position.y;
    if (p_tr->flip == 1)
    {
        n.x += 2 * (p_tr->position.x - n.x) - n.w;
    }
    SDL_FRect intersection;
    if (SDL_GetRectIntersectionFloat(&m, &n, &intersection))
    {
        return true;
    }
    return false;
}

bool collision(SpriteWarp *m_spr, Transform *m_tr)
{
    auto lt = m_spr->lt;
    auto rb = m_spr->rb;
    SDL_FRect m;
    if (lt.has_value() && rb.has_value())
    {
        m.x = lt.value().x;
        m.y = lt.value().y;
        m.w = rb.value().x - lt.value().x;
        m.h = rb.value().y - lt.value().y;
        m.x += m_tr->position.x;
        m.y += m_tr->position.y;
    }
    else
    {
        m.x = m_tr->position.x;
        m.y = m_tr->position.y;
        m.w = m_spr->width;
        m.h = m_spr->height;
    }
    if (m_tr->flip == 1)
    {
        m.x += 2 * (m_tr->position.x - m.x) - m.w;
    }

    auto p_tr = World::registry->try_get<Transform>(Player::ent);
    auto p_cha = World::registry->try_get<Character>(Player::ent);

    SDL_FRect n = p_cha->r;
    n.x += p_tr->position.x;
    n.y += p_tr->position.y;
    if (p_tr->flip == 1)
    {
        n.x += 2 * (p_tr->position.x - n.x) - n.w;
    }
    SDL_FRect intersection;
    if (SDL_GetRectIntersectionFloat(&m, &n, &intersection))
    {
        return true;
    }
    return false;
}

bool collision(Character *m_cha, Transform *m_tr, Attack *n_atk, Transform *n_tr)
{
    SDL_FRect m = m_cha->r;
    m.x += m_tr->position.x;
    m.y += m_tr->position.y;
    if (m_tr->flip == 1)
    {
        m.x += 2 * (m_tr->position.x - m.x) - m.w;
    }

    SDL_FRect n = n_atk->rect;
    n.x += n_tr->position.x;
    n.y += n_tr->position.y;
    if (n_tr->flip == 1)
    {
        n.x += 2 * (n_tr->position.x - n.x) - n.w;
    }
    SDL_FRect intersection;
    if (SDL_GetRectIntersectionFloat(&m, &n, &intersection))
    {
        return true;
    }
    return false;
}

void hit_effect(Attack *atk, SDL_FPoint *head, entt::entity *ent, char type)
{
    auto damage = atk->damage;

    auto &hit = World::registry->emplace_or_replace<Hit>(*ent);
    hit.x = atk->p->x;
    hit.y = atk->p->y;
    hit.damage = damage;

    auto dam = World::registry->try_get<Damage>(*ent);
    dam->damage.push_back({damage, 255, (int)dam->damage.size(), type});
    dam->head = head;

    if (atk->hit)
    {
        auto eff = World::registry->try_get<Effect>(*ent);
        eff->effects.push_back(AnimatedSprite(atk->hit));
    }
}
