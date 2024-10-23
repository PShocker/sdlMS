module;

#include "entt/entt.hpp"
#include <SDL3/SDL.h>

module systems;

import core;
import components;
import commons;

void attack_run()
{
    if (auto atk = World::registry->try_get<Attack>(Player::ent))
    {
        for (auto it = atk->atks.begin(); it != atk->atks.end();)
        {
            auto atkw = &(*it);
            if (atkw->attack)
            {
                player_attack(atkw);
                it = atk->atks.erase(it);
                continue;
            }
            it++;
        }
    }
    if (player_invincible_cooldown <= 0)
    {
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
        {
            auto view = World::registry->view<Trap>();
            for (auto &ent : view)
            {
                auto trap = &view.get<Trap>(ent);
                if (trap_collision(trap, World::registry->try_get<AnimatedSprite>(ent), World::registry->try_get<Transform>(ent)))
                {
                    return;
                }
            }
        }
    }
}

void player_attack(AttackWarp *atkw)
{
    for (auto ent : World::registry->view<Damage>())
    {
        if (ent != Player::ent)
        {
            if (atkw->mobCount <= 0)
            {
                break;
            }
            if (auto mob = World::registry->try_get<Mob>(ent))
            {
                if (!(mob->state == Mob::State::DIE || mob->state == Mob::State::REMOVE))
                {
                    if (attack_mob(atkw, mob, &ent))
                    {
                        atkw->mobCount -= 1;
                    }
                }
            }
            else if (auto npc = World::registry->try_get<Npc>(ent))
            {
                if (attack_npc(atkw, npc, &ent))
                {
                    atkw->mobCount -= 1;
                }
            }
            else if (auto cha = World::registry->try_get<Character>(ent))
            {
                if (cha->invincible_cooldown <= 0 && attack_cha(atkw, cha, &ent))
                {
                    atkw->mobCount -= 1;
                }
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
        AttackWarp atkw;
        atkw.damage = 100;
        atkw.hit = nullptr;
        atkw.p = &m_tr->position;
        hit_effect(&atkw, nullptr, &Player::ent, 1, atkw.damage);
        return true;
    }
    return false;
}

bool trap_collision(Trap *trap, AnimatedSprite *aspr, Transform *t_tr)
{
    auto sprw = aspr->aspr->sprites[aspr->anim_index];
    if (collision(sprw, t_tr))
    {
        AttackWarp atkw;
        atkw.damage = trap->damage;
        atkw.hit = nullptr;
        atkw.p = &t_tr->position;
        hit_effect(&atkw, nullptr, &Player::ent, 1, atkw.damage);
        return true;
    }
    return false;
}

bool attack_mob(AttackWarp *atkw, Mob *mob, entt::entity *ent)
{
    auto animated = mob->a[mob->index];
    auto m_spr = animated->aspr->sprites[animated->anim_index];

    auto p_tr = World::registry->try_get<Transform>(Player::ent);
    auto m_tr = World::registry->try_get<Transform>(*ent);

    if (collision(m_spr, m_tr, atkw, p_tr))
    {
        hit_effect(atkw, &m_spr->head.value(), ent, 0, atkw->damage, atkw->attackCount);
        return true;
    }
    return false;
}

bool attack_npc(AttackWarp *atkw, Npc *npc, entt::entity *ent)
{
    auto animated = npc->a[npc->index];
    auto n_spr = animated->aspr->sprites[animated->anim_index];

    auto p_tr = World::registry->try_get<Transform>(Player::ent);
    auto n_tr = World::registry->try_get<Transform>(*ent);

    if (collision(n_spr, n_tr, atkw, p_tr))
    {
        hit_effect(atkw, &n_spr->head.value(), ent, 0, 0);
        return true;
    }
    return false;
}

bool attack_cha(AttackWarp *atkw, Character *cha, entt::entity *ent)
{
    auto p_tr = World::registry->try_get<Transform>(Player::ent);
    auto n_tr = World::registry->try_get<Transform>(*ent);

    if (collision(cha, n_tr, atkw, p_tr))
    {
        hit_effect(atkw, nullptr, ent, 0, atkw->damage * 10, atkw->attackCount);
        return true;
    }
    return false;
}

bool collision(SpriteWarp *m_spr, Transform *m_tr, AttackWarp *atkw, Transform *p_tr)
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

    SDL_FRect n = atkw->rect;
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

bool collision(Character *m_cha, Transform *m_tr, AttackWarp *n_atkw, Transform *n_tr)
{
    SDL_FRect m = m_cha->r;
    m.x += m_tr->position.x;
    m.y += m_tr->position.y;
    if (m_tr->flip == 1)
    {
        m.x += 2 * (m_tr->position.x - m.x) - m.w;
    }

    SDL_FRect n = n_atkw->rect;
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

void hit_effect(AttackWarp *atkw, SDL_FPoint *head, entt::entity *ent, char type, int damage, int count)
{
    auto hit = World::registry->try_get<Hit>(*ent);
    hit->x = atkw->p->x;
    hit->y = atkw->p->y;
    hit->souw = atkw->souw;
    hit->count += count;
    hit->damage += damage * count;

    for (int i = 0; i < count; i++)
    {
        auto dam = World::registry->try_get<Damage>(*ent);
        dam->damage.push_back({damage, 255, (int)dam->damage.size(), type});
        dam->head = head;

        if (atkw->hit)
        {
            auto eff = World::registry->try_get<Effect>(*ent);
            eff->effects.push_back({nullptr, AnimatedSprite(atkw->hit), Window::dt_now + i * 145});
        }
    }
}
