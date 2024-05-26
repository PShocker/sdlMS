#include "EntitySystem.h"
#include "Components/Player.h"
#include "Entities/String.h"
#include "Entities/NameTag.h"

void EntitySystem::run(World &world)
{
    // 更新三段式传送门状态
    if (world.entity_exist_of_type<Portal>())
    {
        for (auto &[index, por] : world.get_entitys<Portal>())
        {
            update_portal(por, world);
        }
    }
}

void EntitySystem::update_portal(Portal *por, World &world)
{
    if (por->aspr_map.size() == 3)
    {
        if (world.components_exist_of_type<Player>())
        {
            auto player = world.get_components<Player>().begin()->second;
            auto pla_tr = player->get_owner_component<Transform>();

            auto por_tr = por->get_component<Transform>(); // 传送门坐标
            auto aspr = por->get_component<AnimatedSprite>();
            auto dis = distance(por_tr->get_position(), pla_tr->get_position());

            if (dis < 100 && aspr != por->aspr_map[u"portalContinue"])
            {
                aspr = por->aspr_map[u"portalContinue"];
                por->add_component(aspr);
            }
            if (100 <= dis && dis < 150 && aspr != por->aspr_map[u"portalExit"])
            {
                aspr = por->aspr_map[u"portalExit"];
                por->add_component(aspr);
            }
            if (dis >= 150)
            {
                // 距离太远,直接隐藏
                if (aspr != nullptr)
                {
                    por->remove_component<AnimatedSprite>();
                }
            }
        }
    }
}
