module;

#include "entt/entt.hpp"

module systems;

import core;
import components;
import entities;

void world_run()
{
    world_transport();
    world_sort_zindex();
}

void world_sort_zindex()
{
    if (World::zindex)
    {
        World::registry->sort<Transform>([](const auto &lhs, const auto &rhs)
                                         { return lhs.z < rhs.z; });
        World::zindex = false;
    }
}

void world_transport()
{
    if (World::TransPort::id != 0)
    {
        auto r = World::load_map(World::TransPort::id);
        r->destroy(Player::ent);
        auto view = World::registry->view<Portal>();
        for (auto &e : view)
        {
            auto por = &view.get<Portal>(e);
            if (por->pn == World::TransPort::tn)
            {
                auto tr = World::registry->try_get<Transform>(e);
                Player::ent = World::registry->create();
                load_character(tr->position.x, tr->position.y - 10, false, &Player::ent);
                camera_refresh();
                World::TransPort::id = 0;
                Window::tick_delta_time();
                return;
            }
        }
    }
}
