module;

#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include <optional>

module systems;

import core;
import components;
import commons;
import :collision;
import :hit;

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
    if (player_invincible_cooldown <= 0)
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

void attacking(AttackWarp *atkw)
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
                        if (collision(atkw, mob, World::registry->try_get<Transform>(ent)))
                        {
                            hit_effect(atkw, mob, ent);
                            atkw->mobCount -= 1;
                        }
                    }
                }
                else if (auto npc = World::registry->try_get<Npc>(ent))
                {
                    if (collision(atkw, npc, World::registry->try_get<Transform>(ent)))
                    {
                        hit_effect(atkw, npc, ent);
                        atkw->mobCount -= 1;
                    }
                }
                else if (auto cha = World::registry->try_get<Character>(ent))
                {
                    if (cha->invincible_cooldown <= 0 && collision(atkw, cha, World::registry->try_get<Transform>(ent)))
                    {
                        hit_effect(atkw, cha, ent);
                        atkw->mobCount -= 1;
                    }
                }
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
            if (collision(mob, World::registry->try_get<Transform>(ent)))
            {
                AttackWarp atkw(mob, World::registry->try_get<Transform>(ent));
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
        if (collision(trap, World::registry->try_get<AnimatedSprite>(ent), World::registry->try_get<Transform>(ent)))
        {
            AttackWarp atkw(trap, World::registry->try_get<Transform>(ent));
            hit_effect(&atkw, std::nullopt, Player::ent, 1, atkw.damage);
            return true;
        }
    }
    return false;
}
