module;

#include "entt/entt.hpp"

module core;

void World::load_map(int id)
{
    if (registrys.contains(id))
    {
        registry = registrys[id];
    }
    else
    {
        registrys[id] = entt::registry{};
    }
}
