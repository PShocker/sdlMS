module;

#include <SDL3/SDL.h>
#include "entt/entt.hpp"

module systems;

import components;
import commons;
import core;
import entities;
import :move;

void drop_run()
{
    auto view = World::registry->view<Drop>();
    for (auto &ent : view)
    {
        auto dro = &view.get<Drop>(ent);
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
            if (drop_pick(mv, tr, dro, (float)Window::delta_time / 1000))
            {
                // 掉落物被捡起来,且完全透明,销毁
                // World::registry->destroy(ent);
            }
        }
    }
}

bool drop_fall(Move *mv, Transform *tr, float delta_time)
{
    mv->vspeed += delta_time * 2000;
    return move_fall(mv, tr, delta_time, DROP_Z);
}

bool drop_pick(Move *mv, Transform *tr, Drop *dro, float delta_time)
{
    // 默认重力为2000
    // 捡起物品有两阶段,分别为物品向上运动和吸引物品
    auto picker_tr = World::registry->try_get<Transform>(*dro->picker);
    auto dx = (picker_tr->position.x - 10) - tr->position.x;
    mv->hspeed = dx / delta_time / 5;

    if (dro->attract)
    {
        auto dy = picker_tr->position.y - tr->position.y;
        mv->vspeed = dy / delta_time / 15;
    }
    else
    {
        mv->vspeed += delta_time * 2000;
        if (mv->vspeed >= 0)
        {
            dro->attract = true;
        }
    }
    dro->alpha -= delta_time * 320;
    if (dro->alpha <= 0)
    {
        dro->alpha = 0;
        return true;
    }
    move_fall(mv, tr, delta_time, DROP_Z);
    return false;
}
