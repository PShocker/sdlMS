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
        auto mv = World::registry->try_get<Move>(ent);
        auto tr = World::registry->try_get<Transform>(ent);
        drop_fall(mv, tr, (float)Window::delta_time / 1000);
    }
}

bool drop_fall(Move *mv, Transform *tr, float delta_time)
{
    // 默认重力为2000
    mv->vspeed += delta_time * 2000;
    return move_fall(mv, tr, delta_time);
}
