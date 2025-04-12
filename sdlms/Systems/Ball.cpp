#include "Ball.h"
#include "Rect.h"
#include "Move.h"
#include "Collision.h"
#include "PlayerSkill/Common.h"
#include "Core/Core.h"
#include "Commons/Commons.h"
#include "Entities/Entities.h"
#include <SDL3/SDL.h>
#include "entt/entt.hpp"
#include <vector>
#include <numbers>
#include <cfloat>
#include "Systems/Attack.h"

void ball_run()
{
    auto view = World::registry->view<Ball>();
    for (auto ent : view)
    {
        auto ball = World::registry->try_get<Ball>(ent);
        if (ball->track == false)
        {
            // 这类型的ball不跟踪怪物,路径就是直线
            if (ball_no_track(ent, (float)Window::delta_time / 1000))
            {
                World::destory.push_back(ent);
                World::zindex = true;
            }
        }
        if (ball->track && ball->target_point != std::nullopt)
        {
            if (ball_track(ent, (float)Window::delta_time / 1000))
            {
                World::destory.push_back(ent);
                World::zindex = true;
            }
        }
        else if (ball->track && ball->target_point == std::nullopt)
        {
            ball_fall(ent);
        }
    }
}

entt::entity ball_fall(entt::entity src)
{
    auto ball = World::registry->try_get<Ball>(src);
    auto mv = World::registry->try_get<Move>(src);
    auto tr = World::registry->try_get<Transform>(src);

    const Triangle tri = {{-350, -100}, {-350, 100}, {0, 0}};

    auto target = find_closest_attackable_mob(*tr, tri);
    ball_target_point(src, target);
    return target;
}

bool ball_track(entt::entity src, float delta_time)
{
    auto ball = World::registry->try_get<Ball>(src);
    auto mv = World::registry->try_get<Move>(src);
    auto tr = World::registry->try_get<Transform>(src);

    float dx;
    float dy;
    auto target = ball->target;
    if (World::registry->valid(target))
    {
        if (ball_collision(src, target))
        {
            ball_hit(src, target);
            return true;
        }
        auto target_tr = World::registry->try_get<Transform>(target);
        dx = tr->position.x - (target_tr->position.x + ball->target_point.value().x);
        dy = tr->position.y - (target_tr->position.y + ball->target_point.value().y);
    }
    else
    {
        dx = tr->position.x - ball->target_point.value().x;
        dy = tr->position.y - ball->target_point.value().y;
    }
    mv->hspeed = (dx > 0) ? -std::abs(mv->hspeed) : std::abs(mv->hspeed);
    mv->vspeed = -std::abs(mv->hspeed) * dy / std::abs(dx);
    mv->vspeed = mv->vspeed * 0.5;

    tr->rotation = tr->flip ? calculate_angle(ball->point.value(), tr->position)
                            : calculate_angle(tr->position, ball->point.value());

    move_fall(mv, tr, delta_time, 0, false, true);
    if (mv->hspeed == 0 ||
        (!World::registry->valid(target) && std::abs(dx) <= 10) ||
        (!World::registry->valid(target) && ball->destory <= Window::dt_now))
    {
        return true;
    }
    return false;
}

bool ball_no_track(entt::entity src, float delta_time)
{
    auto ball = World::registry->try_get<Ball>(src);
    auto mv = World::registry->try_get<Move>(src);
    auto tr = World::registry->try_get<Transform>(src);

    for (auto ent : World::registry->view<Mob>())
    {
        auto mob = World::registry->try_get<Mob>(ent);
        if (mob->state == Mob::State::DIE || mob->state == Mob::State::REMOVE)
            continue;

        if (!ball->track_hit->contains(ent))
        {
            if (ball_collision(src, ent))
            {
                ball_hit(src, ent);
                ball->track_hit->insert(ent);
            }
        }
    }
    move_fall(mv, tr, delta_time, 0, false, true);
    if (mv->hspeed == 0 || ball->destory <= Window::dt_now)
    {
        return true;
    }
    return false;
}

void ball_target_point(entt::entity src, entt::entity target)
{
    auto ball = World::registry->try_get<Ball>(src);
    if (!World::registry->valid(src) || !World::registry->valid(target))
    {
        auto ball_tr = World::registry->try_get<Transform>(src);
        auto target_point = ball_tr->flip == 1 ? SDL_FPoint{350, 0} : SDL_FPoint{-350, 0};
        ball->target_point = ball_tr->position + target_point;
        return;
    }

    ball->target = target;

    auto target_tr = World::registry->try_get<Transform>(target);
    auto ball_tr = World::registry->try_get<Transform>(src);

    if (auto mob = World::registry->try_get<Mob>(target))
    {
        auto rect = real_rect(mob->rect(), target_tr);
        ball->target_point = closestPointToRect(ball_tr->position, rect) - target_tr->position;
    }
}

void ball_hit(entt::entity src, entt::entity target)
{
    auto ball = World::registry->try_get<Ball>(src);

    auto ski = World::registry->try_get<Skill>(src);
    Attack attack;
    Attack *atk = (ski) ? &(ski->atk.value()) : &attack;

    if (!atk->hit)
        atk->hit = ball->hit;

    if (atk->mobCount >= 1)
    {
        atk->src_point = World::registry->try_get<Transform>(src)->position;
        if (auto mob = World::registry->try_get<Mob>(target))
        {
            atk->mobCount--;
            if (ski && track_no_skill.contains(ski->skiw->id))
            {
                attack_mob(atk, src, target, std::nullopt); // 传递地址，保持原有行为
            }
            else
            {
                attack_mob(atk, src, target, atk->src_point); // 传递地址，保持原有行为
            }
        }
    }
}

bool ball_collision(entt::entity src, entt::entity target)
{
    if (!World::registry->valid(src) || !World::registry->valid(target))
        return false;

    auto ball = World::registry->try_get<Ball>(src);
    auto target_point = SDL_FPoint{0, -30};
    if (ball->target_point.has_value())
    {
        target_point = ball->target_point.value();
    }
    auto target_position = target_point + World::registry->try_get<Transform>(target)->position;
    auto src_position = World::registry->try_get<Transform>(src)->position;
    if (std::abs(src_position.x - target_position.x) <= 10 && std::abs(src_position.y - target_position.y) <= 20)
    {
        return true;
    }
    return false;
}
