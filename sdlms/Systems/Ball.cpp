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
            ball_target(&ent, ball, (float)Window::delta_time / 1000);
        }
        else
        {
            ball_fall(&ent, ball, (float)Window::delta_time / 1000);
        }
    }
    for (auto ent : destory)
    {
        World::registry->destroy(ent);
    }
    destory.clear();
}

bool ball_fall(entt::entity *ent, Ball *ball, float delta_time)
{
    auto mv = World::registry->try_get<Move>(*ent);
    auto tr = World::registry->try_get<Transform>(*ent);
    // 飞行
    move_fall(mv, tr, delta_time, 0, false);
    // 判断前方是否有怪物可以攻击
    // 如果有攻击目标,则改变运动轨迹
    auto ski = World::registry->try_get<Skill>(*ent);
    auto mobCount = ski->ski->infos[ski->level]->mobCount;

    for (auto e : World::registry->view<Damage>())
    {
        if (mobCount > 0 && ball->owner != e)
        {
            if (auto mob = World::registry->try_get<Mob>(e))
            {
                if (!(mob->state == Mob::State::DIE || mob->state == Mob::State::REMOVE))
                {
                    if (ball_target_mob(ent, ball, &e, mobCount > 1))
                    {
                        ball->target = e;
                        mobCount -= 1;
                    }
                }
            }
        }
    }

    return false;
}

bool ball_target_mob(entt::entity *src, Ball *ball, entt::entity *target, bool multiple)
{
    const SDL_FRect rect = {0, 0, 80, 80};
    auto n_tr = World::registry->try_get<Transform>(*src);

    auto mob = World::registry->try_get<Mob>(*target);
    auto animated = mob->a[mob->index];
    auto m_spr = animated->aspr->sprites[animated->anim_index];
    auto m_tr = World::registry->try_get<Transform>(*target);

    if (collision(m_spr->rect.value(), m_tr, rect, n_tr))
    {
        return true;
    }
    return false;
}

bool ball_target(entt::entity *src, Ball *ball, float delta_time)
{
    auto target = ball->target;
    auto t_tr = World::registry->try_get<Transform>(target);
    auto b_tr = World::registry->try_get<Transform>(*src);

    auto mv = World::registry->try_get<Move>(*src);

    b_tr->position = t_tr->position;
    // float dx = t_tr->position.x - b_tr->position.x;
    // mv->hspeed = dx * delta_time;

    // auto dy = t_tr->position.y - b_tr->position.y;
    // mv->vspeed = dy * delta_time;

    // move_fall(mv, b_tr, delta_time, 0, false);

    return false;
}
