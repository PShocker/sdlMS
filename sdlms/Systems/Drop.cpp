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

void drop_run()
{
    static std::vector<entt::entity> destory;
    auto view = World::registry->view<Drop>();
    for (auto &ent : view)
    {
        auto dro = &view.get<Drop>(ent);
        if (dro->destory > Window::dt_now)
        {
            auto mv = World::registry->try_get<Move>(ent);
            auto tr = World::registry->try_get<Transform>(ent);
            if (dro->land == false)
            {
                if (!drop_fall(mv, tr, (float)Window::delta_time / 1000))
                {
                    dro->land = true;
                }
            }
            if (dro->picker)
            {
                drop_pick(mv, tr, dro, (float)Window::delta_time / 1000);
            }
        }
        else
        {
            destory.push_back(ent);
            World::zindex = true;
        }
    }
    for (auto &ent : destory)
    {
        World::registry->destroy(ent);
    }
    destory.clear();
}

bool drop_fall(Move *mv, Transform *tr, float delta_time)
{
    mv->vspeed += delta_time * 1000;
    return move_fall(mv, tr, delta_time, DROP_Z);
}

void drop_pick(Move *mv, Transform *tr, Drop *dro, float delta_time)
{
    // 默认重力为2000
    // 捡起物品有两阶段,分别为物品向上运动和吸引物品
    auto picker_tr = World::registry->try_get<Transform>(*dro->picker);
    auto picker_tr_x = picker_tr->position.x - 10;
    float dx = picker_tr_x - std::lerp(tr->position.x, picker_tr_x, 0.2);

    mv->hspeed = dx / delta_time / 6;

    if (dro->attract)
    {
        auto dy = picker_tr->position.y - tr->position.y;
        mv->vspeed = dy / delta_time / 40;
    }
    else
    {
        mv->vspeed += delta_time * 2000;
        if (mv->vspeed >= 0)
        {
            dro->attract = true;
        }
    }
    move_fall(mv, tr, delta_time, DROP_Z);
    return;
}