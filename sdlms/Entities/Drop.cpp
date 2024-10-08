module;

#include "wz/Property.hpp"
#include <any>
#include "entt/entt.hpp"

module entities;

import components;
import resources;
import core;
import commons;

void load_drop(const std::u16string &id, uint64_t nums, float x, float y, int z)
{
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
    }
    auto ent = World::registry->create();

    World::registry->emplace<Animated>(ent);
    World::registry->emplace<Transform>(ent, (float)x, (float)y, z);

    auto &dro = World::registry->emplace<Drop>(ent);
    dro.aspr = AnimatedSprite(asprw);

    auto &mv = World::registry->emplace<Move>(ent);
    mv.vspeed = -555;
}
