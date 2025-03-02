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
            attack_iterator(&it, ent);
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

void attack_iterator(AttackWarp *atkw, entt::entity ent)
{
    attack_mob(atkw, ent);
    attack_reactor(atkw);
}

void attack_mob(AttackWarp *atkw, entt::entity ent)
{
    auto e_tr = World::registry->try_get<Transform>(ent);
    for (auto ent : World::registry->view<Damage, Mob>())
    {
        if (atkw->mobCount > 0)
        {
            auto mob = World::registry->try_get<Mob>(ent);
            if (!(mob->state == Mob::State::DIE || mob->state == Mob::State::REMOVE))
            {
                auto m_tr = World::registry->try_get<Transform>(ent);

                if (collision(mob->rect(), m_tr, atkw->rect, e_tr))
                {
                    hit_effect(atkw, mob->head(), ent, 0, std::nullopt);
                    atkw->mobCount -= 1;
                }
            }
        }
    }
}

void attack_reactor(AttackWarp *atkw)
{
    if (!World::registry->all_of<Skill>(Player::ent))
    {
        auto p_tr = World::registry->try_get<Transform>(Player::ent);
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
                        if (std::abs(p_tr->position.y - r_tr->position.y) <= 50)
                        {
                            if ((p_tr->flip == 1 && p_tr->position.x <= r_tr->position.x && (r_tr->position.x - p_tr->position.x) <= 95) ||
                                (p_tr->flip == 0 && p_tr->position.x >= r_tr->position.x && (p_tr->position.x - r_tr->position.x) <= 95))
                            {
                                Sound::push(r->sounds[r->index]);
                                atkw->damage = 1;
                                hit_effect(atkw, r->head(), ent, 2, std::nullopt);
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
                atkw.p = m_tr->position;
                hit_effect(&atkw, std::nullopt, Player::ent, 1, std::nullopt);
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
        auto spr = aspr->asprw->sprites[aspr->anim_index];
        if (spr->n->get_child(u"lt") && spr->n->get_child(u"rb"))
        {
            auto lt = dynamic_cast<wz::Property<wz::WzVec2D> *>(spr->n->get_child(u"lt"))->get();
            auto rb = dynamic_cast<wz::Property<wz::WzVec2D> *>(spr->n->get_child(u"rb"))->get();
            auto x = lt.x;
            auto y = lt.y;
            auto w = rb.x - lt.x;
            auto h = rb.y - lt.y;
            auto rect = SDL_FRect{(float)x, (float)y, (float)w, (float)h};

            auto t_tr = World::registry->try_get<Transform>(ent);
            auto p_tr = World::registry->try_get<Transform>(Player::ent);
            auto p_cha = World::registry->try_get<Character>(Player::ent);

            if (collision(rect, t_tr,
                          p_cha->r, p_tr))
            {
                AttackWarp atkw;
                atkw.damage = trap->damage;
                atkw.p = t_tr->position;
                hit_effect(&atkw, std::nullopt, Player::ent, 1, std::nullopt);
                return true;
            }
        }
    }
    return false;
}
