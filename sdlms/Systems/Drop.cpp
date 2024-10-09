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
        if (dro->pick == true)
        {
            if (drop_pick(mv, tr, &dro->aspr, (float)Window::delta_time / 1000))
            {
                // 掉落物被捡起来,且完全透明,销毁
                World::registry->destroy(ent);
            }
        }
    }
}

bool drop_fall(Move *mv, Transform *tr, float delta_time)
{
    // 默认重力为2000
    mv->vspeed += delta_time * 2000;
    return move_fall(mv, tr, DROP_Z, delta_time);
}

bool drop_pick(Move *mv, Transform *tr, AnimatedSprite *aspr, float delta_time)
{
    // 默认重力为2000
    mv->vspeed += delta_time * 2000;
    move_fall(mv, tr, DROP_Z, delta_time);
    aspr->alpha -= 1;
    if (aspr->alpha <= 0)
    {
        return true;
    }
    return false;
}
