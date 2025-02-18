#include "Npc.h"
#include "wz/Property.hpp"
#include "entt/entt.hpp"
#include "Core/Core.h"
#include "Components/Components.h"
#include "Commons/Commons.h"
#include "Resources/Wz.h"
#include "FootHold.h"

void load_npc(wz::Node *node)
{
    auto ent = World::registry->create();

    auto fh = dynamic_cast<wz::Property<int> *>(node->get_child(u"fh"))->get();
    auto foo = fhs[fh];

    auto &mv = World::registry->emplace<Move>(ent);
    mv.rx0 = dynamic_cast<wz::Property<int> *>(node->get_child(u"rx0"))->get();
    mv.rx1 = dynamic_cast<wz::Property<int> *>(node->get_child(u"rx1"))->get();
    mv.foo = foo;

    auto &npc = World::registry->emplace<Npc>(ent);
    auto id = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"id"))->get();
    auto x = dynamic_cast<wz::Property<int> *>(node->get_child(u"x"))->get();
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
            if (key != u"shop")
            {
                auto asprw = AnimatedSpriteWarp::load(val[0]);
                npc.a[key] = AnimatedSprite(asprw);
            }
        }
    }

    if (npc.a.contains(u"stand"))
    {
        npc.index = u"stand";
    }
    else
    {
        npc.index = npc.a.begin()->first;
    }

    World::registry->emplace<Hit>(ent);
    World::registry->emplace<Effect>(ent);
    World::registry->emplace<Damage>(ent);
    World::registry->emplace<Animated>(ent);
    World::registry->emplace<Transform>(ent, (float)x, (float)y, layer * LAYER_Z + NPC_Z);
}