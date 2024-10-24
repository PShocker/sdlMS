module;

#include "wz/Property.hpp"
#include <any>
#include "entt/entt.hpp"

module entities;

import components;
import resources;
import core;
import commons;

void load_drops(std::vector<DropInfo> *drops, float x, float y, int layer)
{
    for (int i = 0; i < drops->size(); i++)
    {
        auto &id = drops->at(i).id;
        auto &nums = drops->at(i).nums;
        AnimatedSpriteWarp *asprw = nullptr;
        if (id == u"09000000")
        {
            // 金币
            if (nums < 49)
            {
                asprw = Drop::mesoicons[Drop::MesoIcon::BRONZE];
            }
            else if (nums < 99)
            {
                asprw = Drop::mesoicons[Drop::MesoIcon::GOLD];
            }
            else if (nums < 999)
            {
                asprw = Drop::mesoicons[Drop::MesoIcon::BUNDLE];
            }
            else
            {
                asprw = Drop::mesoicons[Drop::MesoIcon::BAG];
            }
        }
        else
        {
            // 测试
            asprw = asprw->load(Wz::Character->get_root()->find_from_path(u"Weapon/01382014.img/info/iconRaw"));
        }
        auto ent = World::registry->create();

        World::registry->emplace<Animated>(ent);
        World::registry->emplace<Transform>(ent, (float)x, (float)y, layer * LAYER_Z + DROP_Z);

        auto &dro = World::registry->emplace<Drop>(ent);
        dro.aspr = AnimatedSprite(asprw);

        auto &mv = World::registry->emplace<Move>(ent);
        mv.vspeed = -360;
        mv.hspeed = 0;
    }
    return;
}
