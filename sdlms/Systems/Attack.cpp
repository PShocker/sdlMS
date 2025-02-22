#include "Attack.h"
#include "Collision.h"
#include "Hit.h"
#include "Core/Core.h"

#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include <optional>

void attack_run()
{
    auto view = World::registry->view<Attack>();
    for (auto ent : view)
    {
        auto atk = &view.get<Attack>(ent);
        for (auto &it : atk->atks)
        {
            attack_iterator(&it);
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

void attack_iterator(AttackWarp *atkw)
{
    attack_mob(atkw);
    //  attack_npc(AttackWarp * atkw);
    //  attack_character(AttackWarp * atkw);
    attack_reactor(atkw);
}

void attack_mob(AttackWarp *atkw)
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
}

void attack_npc(AttackWarp *atkw)
{
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
}

void attack_character(AttackWarp *atkw)
{
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

void attack_reactor(AttackWarp *atkw)
{
    if (!World::registry->all_of<Skill>(Player::ent))
    {
        for (auto ent : World::registry->view<Damage, Reactor>())
        {
            if (atkw->mobCount > 0)
            {
                auto r = World::registry->try_get<Reactor>(ent);

                for (auto &e : r->a[r->index].event)
                {
                    if (e.type == 0 && r->hit == false)
                    {
                        // 说明可以进行hit
                        auto r_tr = World::registry->try_get<Transform>(ent);
                        auto p_tr = World::registry->try_get<Transform>(Player::ent);
                        if (std::abs(p_tr->position.y - r_tr->position.y) <= 30)
                        {
                            if ((p_tr->flip == 1 && p_tr->position.x <= r_tr->position.x && (r_tr->position.x - p_tr->position.x) <= 95) ||
                                (p_tr->flip == 0 && p_tr->position.x >= r_tr->position.x && (p_tr->position.x - r_tr->position.x) <= 95))
                            {
                                Sound::push(r->sounds[r->index]);
                                atkw->damage = 1;
                                hit_effect(atkw, r->head(), ent, 2, nullptr);
                                r->hit = true;
                                return;
                            }
                        }
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
        auto aspr = World::registry->try_get<AnimatedSprite>(ent);
        auto spr = aspr->aspr->sprites[aspr->anim_index];
        if (spr->lt.has_value() && spr->rb.has_value())
        {
            auto x = spr->lt.value().x;
            auto y = spr->lt.value().y;
            auto w = spr->rb.value().x - spr->lt.value().x;
            auto h = spr->rb.value().y - spr->lt.value().y;
            auto rect = SDL_FRect{(float)x, (float)y, (float)w, (float)h};

            auto t_tr = World::registry->try_get<Transform>(ent);
            auto p_tr = World::registry->try_get<Transform>(Player::ent);
            auto p_cha = World::registry->try_get<Character>(Player::ent);

            if (collision(rect, t_tr,
                          p_cha->r, p_tr))
            {
                AttackWarp atkw;
                atkw.damage = trap->damage;
                atkw.p = &t_tr->position;
                hit_effect(&atkw, std::nullopt, Player::ent, 1, nullptr);
                return true;
            }
        }
    }
    return false;
}
