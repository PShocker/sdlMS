#include "World.h"
#include "Map.h"
#include "Window.h"
#include "Components/Components.h"
#include "entt/entt.hpp"

// void World::load_map(int id)
// {
//     if (!registrys.contains(id))
//     {
//         // 未加载过地图
//         registrys[id] = entt::registry{};
//         registry = &registrys[id];
//         Map::load(id);
//     }
//     else
//     {
//         // 地图已经加载
//         registry = &registrys[id];
//         Map::id = id;

//         // 切换音乐
//         Map::load_bgm(id);
//     }
//     Window::tick_delta_time();
// }

void World::load_map(int id)
{
    // if (Map::id != 0 && Map::load_mark(id) != Map::load_mark(Map::id))
    // {
    //     Map::clean_up();
    // }
    World::registry->clear();
    // 遍历并删除所有实体
    // 获取实体存储容器并遍历
    // for (auto ent : World::registry->storage<entt::entity>())
    // {
    //     if (ent != Player::ent)
    //     {
    //         World::registry->destroy(ent);
    //     }
    // }
    Map::load(id);
    Window::tick_delta_time();
}
