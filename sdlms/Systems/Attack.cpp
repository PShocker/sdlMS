module;

#include "entt/entt.hpp"
#include <SDL3/SDL.h>

module systems;

import core;
import components;
import commons;
import :collision;

void attack_run()
{
    if (auto atk = World::registry->try_get<Attack>(Player::ent))
    {
        for (auto it = atk->atks.begin(); it != atk->atks.end(); it++)
        {
            auto atkw = &(*it);
            player_attack(atkw);
        }
        atk->atks.clear();
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
            if (atkw->mobCount > 0)
            {
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
}

bool mob_collision(Mob *mob, Transform *m_tr)
{
    auto animated = mob->a[mob->index];
    auto sprw = animated->aspr->sprites[animated->anim_index];

    auto p_tr = World::registry->try_get<Transform>(Player::ent);
    auto p_cha = World::registry->try_get<Character>(Player::ent);

    if (collision(sprw->rect.value(), m_tr, p_cha->r, p_tr))
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

    auto p_tr = World::registry->try_get<Transform>(Player::ent);
    auto p_cha = World::registry->try_get<Character>(Player::ent);

    if (collision(sprw->rect.value(), t_tr, p_cha->r, p_tr))
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

    if (collision(m_spr->rect.value(), m_tr, atkw->rect, p_tr))
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

    if (collision(n_spr->rect.value(), n_tr, atkw->rect, p_tr))
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

    if (collision(cha->r, n_tr, atkw->rect, p_tr))
    {
        hit_effect(atkw, nullptr, ent, 0, atkw->damage * 10, atkw->attackCount);
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
