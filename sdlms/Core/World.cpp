module;

#include "entt/entt.hpp"

module core;

import components;

void World::zindex()
{
    registry.sort<Transform>([](const auto &lhs, const auto &rhs)
                             { return lhs.z < rhs.z; });
}