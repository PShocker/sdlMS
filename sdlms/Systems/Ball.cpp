#include "Ball.h"
#include "Rect.h"
#include "Hit.h"
#include "Move.h"
#include "Collision.h"
#include "Core/Core.h"
#include "Commons/Commons.h"
#include <SDL3/SDL.h>
#include "entt/entt.hpp"
#include <vector>
#include <numbers>
#include <cfloat>

void ball_run()
{
    auto view = World::registry->view<Ball>();
    for (auto ent : view)
    {
        auto ball = World::registry->try_get<Ball>(ent);
        if (ball->track == false)
        {
            // 这类型的ball不跟踪怪物,路径就是直线
            ball_no_track(ent, ball);
        }
        if (ball->target != entt::null && World::registry->valid(ball->target) && ball->track)
        {
            if (ball_track(ent, ball, (float)Window::delta_time / 1000))
            {
                World::destory.push_back(ent);
                World::zindex = true;
            }
        }
        else if (ball->p == std::nullopt && ball->track)
        {
            ball_fall(ent, ball);
        }
        else if (ball_move(ent, ball, (float)Window::delta_time / 1000))
        {
            World::destory.push_back(ent);
            World::zindex = true;
        }
    }
}

entt::entity ball_fall(entt::entity ent, Ball *ball)
{
    auto mv = World::registry->try_get<Move>(ent);
    auto tr = World::registry->try_get<Transform>(ent);
    auto position = tr->position;
    ball->p = position;

    const Triangle tri = {
        {-350, -100},
        {-350, 100},
        {0, 0},
    };
    float min_distance = FLT_MAX;
    entt::entity target = entt::null;
    for (auto e : World::registry->view<Damage, Mob>())
    {
        auto mob = World::registry->try_get<Mob>(e);
        if (!(mob->state == Mob::State::DIE || mob->state == Mob::State::REMOVE))
        {
            auto m_tr = World::registry->try_get<Transform>(e);
            if (collision(tri, tr, mob->rect(), m_tr))
            {
                auto min_distance2 = distance(m_tr->position, position);
                if (min_distance2 < min_distance)
                {
                    target = e;
                    min_distance = min_distance2;
                }
            }
        }
    }
    for (auto e : World::registry->view<Damage, Character>())
    {
        if (e != Player::ent)
        {
            auto cha = World::registry->try_get<Character>(e);
            auto c_tr = World::registry->try_get<Transform>(e);
            if (cha->invincible_cooldown <= 0 && collision(tri, tr, cha->r, c_tr))
            {
                auto min_distance2 = distance(c_tr->position, position);
                if (min_distance2 < min_distance)
                {
                    target = e;
                    min_distance = min_distance2;
                }
            }
        }
    }
    ball->target = target;
    return target;
}

bool ball_track(entt::entity src, Ball *ball, float delta_time)
{
    float dx;
    float dy;
    ball_distance(src, ball->target, dx, dy);
    auto mv = World::registry->try_get<Move>(src);
    if (dx < 0)
    {
        mv->hspeed = -std::abs(mv->hspeed);
    }
    else
    {
        mv->hspeed = std::abs(mv->hspeed);
    }
    if (dy < 0)
    {
        mv->vspeed = -std::abs(mv->hspeed) / std::abs(dx) * delta_time * 1200;
    }
    else if (dy > 0)
    {
        mv->vspeed = std::abs(mv->hspeed) / std::abs(dx) * delta_time * 1200;
    }
    auto tr = World::registry->try_get<Transform>(src);
    move_fall(mv, tr, delta_time, 0, false, true);
    if (mv->hspeed == 0)
    {
        return true;
    }
    // 旋转
    if (ball->rotate.has_value() == false)
    {
        tr->rotation = ball_rotation(ball, tr);
    }
    if (std::abs(dx) <= 10 && std::abs(dy) <= 10)
    {
        ball_hit(src, ball, ball->target);
        return true;
    }
    return false;
}

bool ball_no_track(entt::entity src, Ball *ball)
{
    for (auto e : World::registry->view<Damage, Mob>())
    {
        auto mob = World::registry->try_get<Mob>(e);
        if (!(mob->state == Mob::State::DIE || mob->state == Mob::State::REMOVE))
        {
            if (!ball->track_hit.value().contains(e))
            {
                float dx;
                float dy;
                ball_distance(src, e, dx, dy);
                if (std::abs(dx) <= 10 && std::abs(dy) <= 10)
                {
                    ball_hit(src, ball, e);
                    ball->track_hit.value().insert(e);
                }
            }
        }
    }
    return false;
}

bool ball_move(entt::entity src, Ball *ball, float delta_time)
{
    auto mv = World::registry->try_get<Move>(src);
    auto tr = World::registry->try_get<Transform>(src);
    move_fall(mv, tr, delta_time, 0, false);
    if (ball->destory < Window::dt_now || mv->hspeed == 0)
    {
        return true;
    }
    return false;
}

void ball_hit(entt::entity src, Ball *ball, entt::entity target)
{
    auto ski = World::registry->try_get<Skill>(src);
    AttackWarp *atkw = nullptr;
    if (ski == nullptr)
    {
        atkw = new AttackWarp();
    }
    else
    {
        atkw = &ski->atkw.value();
    }
    if (atkw->hit == nullptr)
    {
        atkw->hit = ball->hit;
    }
    if (atkw->mobCount >= 1)
    {
        atkw->p = &World::registry->try_get<Transform>(src)->position;
        if (auto mob = World::registry->try_get<Mob>(target))
        {
            hit_effect(atkw, mob->head(), target, 0, atkw->p);
        }
        else if (auto cha = World::registry->try_get<Character>(target))
        {
            hit_effect(atkw, target, 1, atkw->p);
        }
        atkw->mobCount--;
    }
    if (ski == nullptr)
    {
        delete atkw;
    }
}

bool ball_distance(entt::entity src, entt::entity target, float &dx, float &dy)
{
    if (World::registry->valid(src) && World::registry->valid(target))
    {
        auto t_tr = World::registry->try_get<Transform>(target);
        auto b_tr = World::registry->try_get<Transform>(src);

        auto mv = World::registry->try_get<Move>(src);

        SDL_FPoint t_position;
        SDL_FRect t_rect;
        if (auto mob = World::registry->try_get<Mob>(target))
        {
            t_rect = mob->rect();
            auto rect = real_rect(t_rect, t_tr);
            t_position = closestPointToRect(b_tr->position, rect);
        }
        else if (auto cha = World::registry->try_get<Character>(target))
        {
            t_rect = cha->r;
            auto rect = real_rect(t_rect, t_tr);
            t_position = closestPointToRect(b_tr->position, rect);
        }
        dx = t_position.x - b_tr->position.x;
        dy = t_position.y - b_tr->position.y;
        return true;
    }
    else
    {
        return false;
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
