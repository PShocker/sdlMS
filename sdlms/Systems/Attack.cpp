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
        for (auto it = atk->atks.begin(); it != atk->atks.end(); it++)
        {
            auto atkw = &(*it);
            attacking(atkw);
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
                if (collision(atkw, mob, World::registry->try_get<Transform>(ent)))
                {
                    hit_effect(atkw, mob, ent, nullptr);
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
            if (collision(atkw, npc, World::registry->try_get<Transform>(ent)))
            {
                hit_effect(atkw, npc, ent, nullptr);
                atkw->mobCount -= 1;
            }
        }
    }
    for (auto ent : World::registry->view<Damage, Character>())
    {
        if (atkw->mobCount > 0 && ent != Player::ent)
        {
            auto cha = World::registry->try_get<Character>(ent);
            if (cha->invincible_cooldown <= 0 && collision(atkw, cha, World::registry->try_get<Transform>(ent)))
            {
                hit_effect(atkw, cha, ent, nullptr);
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
            if (collision(mob, m_tr))
            {
                AttackWarp atkw;
                atkw.damage = 100;
                atkw.p = &m_tr->position;
                hit_effect(&atkw, std::nullopt, Player::ent, 1, atkw.damage);
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
        if (collision(trap, World::registry->try_get<AnimatedSprite>(ent), t_tr))
        {
            AttackWarp atkw;
            atkw.damage = trap->damage;
            atkw.p = &t_tr->position;
            hit_effect(&atkw, std::nullopt, Player::ent, 1, atkw.damage);
            return true;
        }
    }
    return false;
}
