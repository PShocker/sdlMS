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
            if (ball_track(ent, ball, (float)Window::delta_time / 1000))
            {
                ball_hit(ent, ball->target);
                destory.push_back(ent);
            }
        }
        else if (ball->p == std::nullopt)
        {
            ball_fall(ent, ball, (float)Window::delta_time / 1000);
        }
        else if (ball_move(ent, ball, (float)Window::delta_time / 1000))
        {
            destory.push_back(ent);
        }
    }
    for (auto ent : destory)
    {
        World::registry->destroy(ent);
        World::zindex = true;
    }
    destory.clear();
}

bool ball_fall(entt::entity ent, Ball *ball, float delta_time)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    auto position = tr->position;
    ball->p = position;

    const Triangle tri = {
        {-300, -90},
        {-300, 120},
        {0, -30},
    };

    for (auto e : World::registry->view<Damage>())
    {
        if (ball->owner != e)
        {
            if (auto mob = World::registry->try_get<Mob>(e))
            {
                if (!(mob->state == Mob::State::DIE || mob->state == Mob::State::REMOVE))
                {
                    auto m_tr = World::registry->try_get<Transform>(e);
                    if (collision(mob, m_tr, tri, tr))
                    {
                        ball->target = e;
                    }
                }
            }
            else if (auto cha = World::registry->try_get<Character>(e))
            {
                auto c_tr = World::registry->try_get<Transform>(e);
                if (cha->invincible_cooldown <= 0 && collision(cha, c_tr, tri, tr))
                {
                    ball->target = e;
                }
            }
        }
    }
    return false;
}

bool ball_track(entt::entity src, Ball *ball, float delta_time)
{
    auto target = ball->target;
    auto t_tr = World::registry->try_get<Transform>(target);
    auto b_tr = World::registry->try_get<Transform>(src);

    auto mv = World::registry->try_get<Move>(src);

    SDL_FPoint t_position;
    SDL_FRect t_rect;
    if (auto mob = World::registry->try_get<Mob>(target))
    {
        t_rect = mob->rect();
        auto rect = mob->rect();
        rect.x += t_tr->position.x;
        rect.y += t_tr->position.y;
        if (t_tr->flip == 1)
        {
            rect.x += 2 * (t_tr->position.x - rect.x) - rect.w;
        }
        t_position = closestPointToRect(b_tr->position, rect);
    }
    else if (auto cha = World::registry->try_get<Character>(target))
    {
        t_rect = cha->r;
        auto rect = cha->r;
        rect.x += t_tr->position.x;
        rect.y += t_tr->position.y;
        if (t_tr->flip == 1)
        {
            rect.x += 2 * (t_tr->position.x - rect.x) - rect.w;
        }
        t_position = closestPointToRect(b_tr->position, rect);
    }

    float dx = t_position.x - b_tr->position.x;
    if (dx < 0)
    {
        mv->hspeed = -std::abs(mv->hspeed);
    }
    else
    {
        mv->hspeed = std::abs(mv->hspeed);
    }

    float dy = t_tr->position.y - b_tr->position.y;
    // float dy = target_y - std::lerp(b_tr->position.y, target_y, 0.4);
    mv->vspeed = dy / delta_time / 10;

    move_fall(mv, b_tr, delta_time, 0, false);

    // 旋转
    b_tr->rotation = ball_rotation(ball, b_tr);

    const SDL_FRect b_rect = {0, 0, 10, 10};

    return collision(t_rect, t_tr, b_rect, b_tr);
}

bool ball_move(entt::entity ent, Ball *ball, float delta_time)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    move_fall(mv, tr, delta_time, 0, false);
    if (ball->destory > Window::dt_now)
    {
        return false;
    }
    return true;
}

void ball_hit(entt::entity src, entt::entity target)
{
    auto ski = World::registry->try_get<Skill>(src);
    AttackWarp atkw(ski);
    atkw.p = &World::registry->try_get<Transform>(src)->position;
    if (auto mob = World::registry->try_get<Mob>(target))
    {
        hit_effect(&atkw, mob, target, atkw.p);
    }
    else if (auto cha = World::registry->try_get<Character>(target))
    {
        hit_effect(&atkw, cha, target, atkw.p);
    }
}

float ball_rotation(Ball *ball, Transform *tr)
{
    auto x1 = ball->p.value().x;
    auto y1 = ball->p.value().y;

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
