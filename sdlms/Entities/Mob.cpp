module;

#include "wz/Property.hpp"
#include "entt/entt.hpp"

module entities;

import core;
import components;
import :foothold;
import resources;
import commons;

void load_mob(wz::Node *node)
{
    auto ent = World::registry->create();

    auto fh = dynamic_cast<wz::Property<int> *>(node->get_child(u"fh"))->get();
    auto foo = fhs[fh];

    auto &mv = World::registry->emplace<Move>(ent);
    mv.rx0 = dynamic_cast<wz::Property<int> *>(node->get_child(u"rx0"))->get();
    mv.rx1 = dynamic_cast<wz::Property<int> *>(node->get_child(u"rx1"))->get();
    mv.foo = foo;

    auto &mob = World::registry->emplace<Mob>(ent);
    auto id = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"id"))->get();
    auto x = dynamic_cast<wz::Property<int> *>(node->get_child(u"x"))->get();

    // 从fh获取y,layer
    auto y = foo->get_y(x).value();
    auto layer = foo->page;

    node = Wz::Mob->get_root()->find_from_path(id + u".img");
    // 排除 link
    while (node->find_from_path(u"info/link") != nullptr)
    {
        auto link = dynamic_cast<wz::Property<wz::wzstring> *>(node->find_from_path(u"info/link"))->get();
        node = Wz::Mob->get_root()->find_from_path(link + u".img");
    }
    for (auto &[key, val] : node->get_children())
    {
        if (key != u"info")
        {
            mob.a[key] = new Animated(val[0]);
        }
        else
        {
            // key = info
            auto speed = 0;
            if (val[0]->get_child(u"speed") != nullptr)
            {
                speed = dynamic_cast<wz::Property<int> *>(val[0]->get_child(u"speed"))->get();
            }
            mv.hspeed = (float)(speed + 100) / 100 * 125;
        }
    }
    mob.index = mob.a.begin()->first;

    World::registry->emplace<Transform>(ent, (float)x, (float)y, layer * LAYER_Z + MOB_Z);
}