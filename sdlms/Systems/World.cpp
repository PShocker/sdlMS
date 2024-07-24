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
        FootHold::fhs = FootHold::fhs_cache[World::TransPort::id];
        Portal::pors = Portal::pors_cache[World::TransPort::id];
        r->destroy(Player::ent);

       
        auto tr = Portal::pors[World::TransPort::tn];

        load_character(tr.x, tr.y - 10);

        camera_refresh();

        World::TransPort::id = 0;
    }
}
