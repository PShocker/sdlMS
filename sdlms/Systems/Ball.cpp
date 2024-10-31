module;

#include <SDL3/SDL.h>
#include "entt/entt.hpp"
#include <vector>
#include <numbers>

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
    const SDL_FRect rect = {-180, -80, 120, 160};

    for (auto e : World::registry->view<Damage>())
    {
        if (mobCount > 0 && ball->owner != e)
        {
            if (auto mob = World::registry->try_get<Mob>(e))
            {
                if (!(mob->state == Mob::State::DIE || mob->state == Mob::State::REMOVE))
                {
                    auto m_tr = World::registry->try_get<Transform>(e);
                    if (collision(mob, m_tr, rect, tr))
                    {
                        ball->target = e;
                        ball->p = m_tr->position;
                        mobCount -= 1;
                    }
                }
            }
            else if (auto cha = World::registry->try_get<Character>(e))
            {
                auto c_tr = World::registry->try_get<Transform>(e);
                if (collision(cha, c_tr, rect, tr))
                {
                    ball->target = e;
                    ball->p = c_tr->position;
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
    else if (auto cha = World::registry->try_get<Character>(target))
    {
        if (t_tr->flip)
        {
            center_x = -cha->r.w / 2;
        }
        else
        {
            center_x = cha->r.w / 2;
        }
        center_y = cha->r.h / 2;
        t_rect = cha->r;
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

    auto target_y = t_tr->position.y - center_y;
    float dy = target_y - b_tr->position.y;
    // float dy = target_y - std::lerp(b_tr->position.y, target_y, 0.4);
    mv->vspeed = dy / delta_time / std::abs(dx);

    move_fall(mv, b_tr, delta_time, 0, false);

    // 旋转
    // b_tr->rotation = 90;
    b_tr->rotation = ball_rotation(ball, b_tr);

    const SDL_FRect b_rect = {-10, -5, 10, 10};

    return collision(t_rect, t_tr, b_rect, b_tr);
}

void ball_hit(entt::entity src, entt::entity target)
{
    auto ski = World::registry->try_get<Skill>(src);
    AttackWarp atkw(ski);
    atkw.p = &World::registry->try_get<Transform>(src)->position;
    if (auto mob = World::registry->try_get<Mob>(target))
    {
        hit_effect(&atkw, mob, target);
    }
    else if (auto cha = World::registry->try_get<Character>(target))
    {
        hit_effect(&atkw, cha, target);
    }
}

float ball_rotation(Ball *ball, Transform *tr)
{
    auto x1 = ball->p.x;
    auto y1 = ball->p.y;

    auto x2 = tr->position.x;
    auto y2 = tr->position.y;

    double dy = y2 - y1;
    double dx = x2 - x1;

    // 避免除以零
    if (dx == 0)
    {
        return (dy > 0) ? 90.0 : 270.0; // 垂直的情况
    }

    // 计算夹角（弧度）
    double angle_rad = std::atan(dy / dx);

    // 转换为度
    double angle_deg = angle_rad * (180.0 / std::numbers::pi);
    if (angle_deg < 0)
    {
        angle_deg += 360.0;
    }
    return angle_deg;
}
