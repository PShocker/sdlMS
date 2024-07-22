module;

#include "entt/entt.hpp"

module systems;

import core;
import components;

void world_run()
{
    world_sort();
}

void world_sort()
{
    if (World::zsort)
    {
        World::registry.sort<Transform>([](const auto &lhs, const auto &rhs)
                                        { return lhs.z < rhs.z; });
        World::zsort = false;
    }
}
