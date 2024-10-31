module;

#include <SDL3/SDL.h>
#include "entt/entt.hpp"
#include <vector>

module systems;

import components;
import commons;
import core;
import entities;
import :move;

void ball_run()
{
    static std::vector<entt::entity> destory;
    auto view = World::registry->view<Ball>();
    for (auto ent : view)
    {
        auto ball = World::registry->try_get<Ball>(ent);
        if (ball->target != entt::null && World::registry->valid(ball->target))
        {
            if (ball_target(ent, ball, (float)Window::delta_time / 1000))
            {
                ball_hit(ent, ball->target);
                destory.push_back(ent);
                World::zindex = true;
            }
        }
        else
        {
            ball_fall(ent, ball, (float)Window::delta_time / 1000);
        }
    }
    for (auto ent : destory)
    {
        World::registry->destroy(ent);
    }
    destory.clear();
}

bool ball_fall(entt::entity ent, Ball *ball, float delta_time)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    // 飞行
    move_fall(mv, tr, delta_time, 0, false);
    // 判断前方是否有怪物可以攻击
    // 如果有攻击目标,则改变运动轨迹
    auto ski = World::registry->try_get<Skill>(ent);
    auto mobCount = ski->ski->infos[ski->level]->mobCount;
    const SDL_FRect rect = {-120, 0, 80, 80};

    for (auto e : World::registry->view<Damage>())
    {
        if (mobCount > 0 && ball->owner != e)
        {
            if (auto mob = World::registry->try_get<Mob>(e))
            {
                if (!(mob->state == Mob::State::DIE || mob->state == Mob::State::REMOVE))
                {
                    if (collision(mob, World::registry->try_get<Transform>(e), rect, tr))
                    {
                        ball->target = e;
                        mobCount -= 1;
                    }
                }
            }
            else if (auto npc = World::registry->try_get<Npc>(e))
            {
                if (collision(npc, World::registry->try_get<Transform>(e), rect, tr))
                {
                    ball->target = e;
                    mobCount -= 1;
                }
            }
            else if (auto cha = World::registry->try_get<Npc>(e))
            {
                if (collision(cha, World::registry->try_get<Transform>(e), rect, tr))
                {
                    ball->target = e;
                    mobCount -= 1;
                }
            }
        }
    }

    return false;
}

bool ball_target(entt::entity src, Ball *ball, float delta_time)
{
    auto target = ball->target;
    auto t_tr = World::registry->try_get<Transform>(target);
    auto b_tr = World::registry->try_get<Transform>(src);

    auto mv = World::registry->try_get<Move>(src);

    float center_x = 0;
    float center_y = 0;
    SDL_FRect t_rect;
    if (auto mob = World::registry->try_get<Mob>(target))
    {
        auto animated = mob->a[mob->index];
        auto m_spr = animated->aspr->sprites[animated->anim_index];
        if (t_tr->flip)
        {
            center_x = -m_spr->rect.value().w / 2;
        }
        else
        {
            center_x = m_spr->rect.value().w / 2;
        }
        center_y = m_spr->rect.value().h / 2;
        t_rect = m_spr->rect.value();
    }

    float dx = (t_tr->position.x - center_x) - b_tr->position.x;
    if (dx < 0)
    {
        mv->hspeed = -std::abs(mv->hspeed);
    }
    else
    {
        mv->hspeed = std::abs(mv->hspeed);
    }

    auto dy = (t_tr->position.y - center_y) - b_tr->position.y;
    if (dy < 0)
    {
        mv->vspeed = -std::abs(mv->hspeed);
    }
    else
    {
        mv->vspeed = std::abs(mv->hspeed);
    }

    move_fall(mv, b_tr, delta_time, 0, false);

    const SDL_FRect b_rect = {0, 0, 40, 80};

    return collision(t_rect, t_tr, b_rect, b_tr);
}

void ball_hit(entt::entity src, entt::entity target)
{
    auto ski = World::registry->try_get<Skill>(src);
    AttackWarp atkw(ski);
    if (auto mob = World::registry->try_get<Mob>(target))
    {
        hit_effect(&atkw, mob, target);
    }
    else if (auto npc = World::registry->try_get<Npc>(target))
    {
        hit_effect(&atkw, npc, target);
    }
    else if (auto cha = World::registry->try_get<Npc>(target))
    {
        hit_effect(&atkw, cha, target);
    }
}
