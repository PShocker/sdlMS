#include "Attack.h"
#include "Collision.h"
#include "Hit.h"
#include "Core/Core.h"

#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include <optional>

void attack_run()
{
    if (auto atk = World::registry->try_get<Attack>(Player::ent))
    {
        for (auto &it : atk->atks)
        {
            attacking(&it);
        }
        atk->atks.clear();
    }
    if (auto cha = World::registry->try_get<Character>(Player::ent))
    {
        if (cha->invincible_cooldown <= 0)
        {
            if (mob_attack())
            {
                return;
            }
            else if (trap_attack())
            {
                return;
            }
        }
    }
}

void attacking(AttackWarp *atkw)
{
    for (auto ent : World::registry->view<Damage, Mob>())
    {
        if (atkw->mobCount > 0)
        {
            auto mob = World::registry->try_get<Mob>(ent);
            if (!(mob->state == Mob::State::DIE || mob->state == Mob::State::REMOVE))
            {
                auto m_tr = World::registry->try_get<Transform>(ent);
                auto p_tr = World::registry->try_get<Transform>(Player::ent);

                if (collision(mob->rect(), m_tr, atkw->rect, p_tr))
                {
                    hit_effect(atkw, mob->head(), ent, 0, nullptr);
                    atkw->mobCount -= 1;
                }
            }
        }
    }
    for (auto ent : World::registry->view<Damage, Npc>())
    {
        if (atkw->mobCount > 0)
        {
            auto npc = World::registry->try_get<Npc>(ent);
            auto n_tr = World::registry->try_get<Transform>(ent);
            auto p_tr = World::registry->try_get<Transform>(Player::ent);

            if (collision(npc->rect(), n_tr, atkw->rect, p_tr))
            {
                hit_effect(atkw, npc->head(), ent, 2, nullptr);
                atkw->mobCount -= 1;
            }
        }
    }
    for (auto ent : World::registry->view<Damage, Character>())
    {
        if (atkw->mobCount > 0 && ent != Player::ent)
        {
            auto cha = World::registry->try_get<Character>(ent);
            auto c_tr = World::registry->try_get<Transform>(ent);
            auto p_tr = World::registry->try_get<Transform>(Player::ent);

            if (cha->invincible_cooldown <= 0 && collision(cha->r, c_tr, atkw->rect, p_tr))
            {
                hit_effect(atkw, ent, 1, nullptr);
                atkw->mobCount -= 1;
            }
        }
    }
}

bool mob_attack()
{
    auto view = World::registry->view<Mob>();
    for (auto ent : view)
    {
        auto mob = &view.get<Mob>(ent);
        if (!(mob->state == Mob::State::DIE || mob->state == Mob::State::REMOVE))
        {
            auto m_tr = World::registry->try_get<Transform>(ent);
            auto p_tr = World::registry->try_get<Transform>(Player::ent);
            auto p_cha = World::registry->try_get<Character>(Player::ent);

            if (collision(mob->rect(), m_tr, p_cha->r, p_tr))
            {
                AttackWarp atkw;
                atkw.damage = 100;
                atkw.p = &m_tr->position;
                hit_effect(&atkw, std::nullopt, Player::ent, 1, nullptr);
                return true;
            }
        }
    }
    return false;
}

bool trap_attack()
{
    auto view = World::registry->view<Trap>();
    for (auto ent : view)
    {
        auto trap = &view.get<Trap>(ent);
        auto t_tr = World::registry->try_get<Transform>(ent);
        auto p_tr = World::registry->try_get<Transform>(Player::ent);
        auto p_cha = World::registry->try_get<Character>(Player::ent);

        if (collision(trap->rect(World::registry->try_get<AnimatedSprite>(ent)),
                      t_tr, p_cha->r, p_tr))
        {
            AttackWarp atkw;
            atkw.damage = trap->damage;
            atkw.p = &t_tr->position;
            hit_effect(&atkw, std::nullopt, Player::ent, 1, nullptr);
            return true;
        }
    }
    return false;
}
