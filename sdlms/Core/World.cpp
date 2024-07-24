module;

#include "entt/entt.hpp"

module core;

entt::registry* World::load_map(int id)
{
    auto r = registry;
    if (!registrys.contains(id))
    {
        // 未加载过地图
        registrys[id] = entt::registry{};
        registry = &registrys[id];
        Map::load(id);
    }
    else
    {
        // 地图已经加载
        registry = &registrys[id];
    }
    return r;
}
