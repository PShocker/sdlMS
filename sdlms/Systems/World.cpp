module;

#include "entt/entt.hpp"

module systems;

import core;
import components;

void world_run()
{
    world_sort_zindex();
}

void world_sort_zindex()
{
    if (World::zsort)
    {
        World::registry.sort<Transform>([](const auto &lhs, const auto &rhs)
                                        { return lhs.z < rhs.z; });
        World::zsort = false;
    }
}
