module;

#include "entt/entt.hpp"

module systems;

import core;
import components;

void world_run()
{
    world_transport();
    world_sort_zindex();
}

void world_sort_zindex()
{
    if (World::zindex)
    {
        World::registry.sort<Transform>([](const auto &lhs, const auto &rhs)
                                        { return lhs.z < rhs.z; });
        World::zindex = false;
    }
}

void world_transport()
{
    if (World::TransPort::id != 0)
    {

        World::TransPort::id = 0;
    }
}
