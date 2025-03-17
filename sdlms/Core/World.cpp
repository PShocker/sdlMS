#include "World.h"
#include "Map.h"
#include "Window.h"
#include "UI/UI.h"
#include "Components/Components.h"
#include "entt/entt.hpp"

void World::load_map(int id)
{
    // if (Map::id != 0 && Map::load_mark(id) != Map::load_mark(Map::id))
    // {
    //     Map::clean_up();
    // }
    // World::registry->clear();
    // 遍历并删除所有实体
    // 获取实体存储容器并遍历
    for (auto ent : World::registry->storage<entt::entity>())
    {
        if (ent == Player::ent)
        {
            continue;
        }
        // 跳过自己的召唤物
        else if (auto sum = World::registry->try_get<Summon>(ent))
        {
            if (sum->owner == Player::ent)
            {
                continue;
            }
        }
        // 跳过自己的宠物
        else if (auto pet = World::registry->try_get<Pet>(ent))
        {
            continue;
        }
        // 跳过右上角UIBuff
        else if (auto uib = World::registry->try_get<UIBuff>(ent))
        {
            continue;
        }
        World::registry->destroy(ent);
    }
    Map::load(id);
    Window::tick_delta_time();
}
