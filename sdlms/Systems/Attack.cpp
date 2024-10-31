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
            attacking(atkw);
        }
        atk->atks.clear();
    }
    if (player_invincible_cooldown <= 0)
    {
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
                        hit_effect(&atkw, nullptr, Player::ent, 1, atkw.damage);
                        return;
                    }
                }
            }
        }
        {
            auto view = World::registry->view<Trap>();
            for (auto ent : view)
            {
                auto trap = &view.get<Trap>(ent);
                if (collision(trap, World::registry->try_get<AnimatedSprite>(ent), World::registry->try_get<Transform>(ent)))
                {
                    AttackWarp atkw(trap, World::registry->try_get<Transform>(ent));
                    hit_effect(&atkw, nullptr, Player::ent, 1, atkw.damage);
                    return;
                }
            }
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

void hit_effect(AttackWarp *atkw, SDL_FPoint *head, entt::entity ent, char type, int damage, int count)
{
    auto hit = World::registry->try_get<Hit>(ent);
    hit->x = atkw->p->x;
    hit->y = atkw->p->y;
    hit->souw = atkw->souw;
    hit->count += count;
    hit->damage += damage * count;

    for (int i = 0; i < count; i++)
    {
        auto dam = World::registry->try_get<Damage>(ent);
        dam->damage.push_back({damage, 255, (int)dam->damage.size(), type});
        dam->head = head;

        if (atkw->hit)
        {
            auto eff = World::registry->try_get<Effect>(ent);
            eff->effects.push_back({nullptr, AnimatedSprite(atkw->hit), Window::dt_now + i * 145});
        }
    }
}

void hit_effect(AttackWarp *atkw, Mob *mob, entt::entity ent)
{
    auto animated = mob->a[mob->index];
    auto m_spr = animated->aspr->sprites[animated->anim_index];
    hit_effect(atkw, &m_spr->head.value(), ent, 0, atkw->damage, atkw->attackCount);
}

void hit_effect(AttackWarp *atkw, Npc *npc, entt::entity ent)
{
    auto animated = npc->a[npc->index];
    auto n_spr = animated->aspr->sprites[animated->anim_index];
    hit_effect(atkw, &n_spr->head.value(), ent, 0, 0);
}

void hit_effect(AttackWarp *atkw, Character *cha, entt::entity ent)
{
    hit_effect(atkw, nullptr, ent, 0, atkw->damage * 10, atkw->attackCount);
}
