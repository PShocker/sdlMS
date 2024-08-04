module;

#include "wz/Property.hpp"
#include "entt/entt.hpp"

module entities;

import core;
import components;
import :foothold;
import resources;
import commons;

void load_npc(wz::Node *node)
{
    auto ent = World::registry->create();

    auto &mv = World::registry->emplace<Move>(ent);
    mv.rx0 = dynamic_cast<wz::Property<int> *>(node->get_child(u"rx0"))->get();
    mv.rx1 = dynamic_cast<wz::Property<int> *>(node->get_child(u"rx1"))->get();

    auto &npc = World::registry->emplace<Npc>(ent);
    auto id = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"id"))->get();
    auto x = dynamic_cast<wz::Property<int> *>(node->get_child(u"x"))->get();
    auto fh = dynamic_cast<wz::Property<int> *>(node->get_child(u"fh"))->get();
    auto foo = fhs[fh];
    // 从fh获取y,layer
    auto y = foo->get_y(x).value();
    auto layer = foo->page;

    node = Wz::Npc->get_root()->find_from_path(id + u".img");
    // 排除 link
    while (node->find_from_path(u"info/link") != nullptr)
    {
        auto link = dynamic_cast<wz::Property<wz::wzstring> *>(node->find_from_path(u"info/link"))->get();
        node = Wz::Npc->get_root()->find_from_path(link + u".img");
    }
    for (auto &[key, val] : node->get_children())
    {
        if (key != u"info")
        {
            npc.a[key] = new AnimatedSprite(val[0]);
        }
    }
    npc.index = npc.a.begin()->first;

    World::registry->emplace<Effect>(ent);
    World::registry->emplace<Animated>(ent);
    World::registry->emplace<Transform>(ent, (float)x, (float)y, layer * LAYER_Z + NPC_Z);
}