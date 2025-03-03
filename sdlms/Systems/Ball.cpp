#include "Ball.h"
#include "Rect.h"
#include "Hit.h"
#include "Move.h"
#include "Collision.h"
#include "Core/Core.h"
#include "Commons/Commons.h"
#include "Entities/Entities.h"
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
        if (ball->track && ball->target != entt::null && World::registry->valid(ball->target))
        {
            if (ball_track(ent, ball, (float)Window::delta_time / 1000))
            {
                World::destory.push_back(ent);
                World::zindex = true;
            }
        }
        else if (ball->track && ball->p == std::nullopt)
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

    ball->p = tr->position;

    const Triangle tri = {{-350, -100}, {-350, 100}, {0, 0}};
    float min_distance = FLT_MAX;
    entt::entity target = entt::null;

    for (auto e : World::registry->view<Damage, Mob>())
    {
        auto mob = World::registry->try_get<Mob>(e);
        if (mob->state == Mob::State::DIE || mob->state == Mob::State::REMOVE)
            continue;

        auto m_tr = World::registry->try_get<Transform>(e);
        if (!collision(tri, tr, mob->rect(), m_tr))
            continue;

        float min_distance2 = squared_distance(m_tr->position, tr->position);
        if (min_distance2 < min_distance)
        {
            target = e;
            min_distance = min_distance2;
        }
    }

    ball->target = target;
    return target;
}

bool ball_track(entt::entity src, Ball *ball, float delta_time)
{
    auto mv = World::registry->try_get<Move>(src);
    auto tr = World::registry->try_get<Transform>(src);

    float dx, dy;
    if (!ball_distance(src, ball->target, dx, dy))
        return false;

    mv->hspeed = (dx < 0) ? -std::abs(mv->hspeed) : std::abs(mv->hspeed);

    mv->vspeed = std::abs(mv->hspeed) * dy / std::abs(dx);
    mv->vspeed = mv->vspeed * 0.35;
    mv->vspeed = std::clamp(mv->vspeed, -std::abs(mv->hspeed), std::abs(mv->hspeed));

    move_fall(mv, tr, delta_time, 0, false, true);
    if (mv->hspeed == 0)
        return true;

    if (!ball->rotate)
    {
        tr->rotation = tr->flip ? calculate_angle(ball->p.value(), tr->position)
                                : calculate_angle(tr->position, ball->p.value());
    }

    if (std::abs(dx) <= 15 && std::abs(dy) <= 20)
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
        if (mob->state == Mob::State::DIE || mob->state == Mob::State::REMOVE)
            continue;

        if (!ball->track_hit->contains(e))
        {
            float dx, dy;
            ball_distance(src, e, dx, dy);
            if (std::abs(dx) <= 15 && std::abs(dy) <= 15)
            {
                ball_hit(src, ball, e);
                ball->track_hit->insert(e);
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
    return (ball->destory < Window::dt_now || mv->hspeed == 0);
}

void ball_hit(entt::entity src, Ball *ball, entt::entity target)
{
    auto ski = World::registry->try_get<Skill>(src);
    Attack atk = (ski) ? ski->atk.value() : Attack(); // 使用值对象

    if (!atk.hit)
        atk.hit = ball->hit;

    if (atk.mobCount >= 1)
    {
        atk.p = World::registry->try_get<Transform>(src)->position;
        if (auto mob = World::registry->try_get<Mob>(target))
        {
            if (ski && track_no_skill.contains(ski->skiw->id))
            {
                hit_effect(&atk, mob->head(), target, 0, std::nullopt); // 传递地址，保持原有行为
            }
            else
            {
                hit_effect(&atk, mob->head(), target, 0, atk.p); // 传递地址，保持原有行为
            }
        }
        atk.mobCount--;
    }
}

bool ball_distance(entt::entity src, entt::entity target, float &dx, float &dy)
{
    if (!World::registry->valid(src) || !World::registry->valid(target))
        return false;

    auto t_tr = World::registry->try_get<Transform>(target);
    auto b_tr = World::registry->try_get<Transform>(src);

    SDL_FPoint t_position;
    if (auto mob = World::registry->try_get<Mob>(target))
    {
        auto rect = real_rect(mob->rect(), t_tr);
        t_position = closestPointToRect(b_tr->position, rect);
    }
    else
    {
        return false;
    }

    dx = t_position.x - b_tr->position.x;
    dy = t_position.y - b_tr->position.y;
    return true;
}