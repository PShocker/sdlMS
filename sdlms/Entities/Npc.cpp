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

    npc.id = id;

    node = Wz::Npc->get_root()->find_from_path(npc.id + u".img");
    // 排除 link
    while (node->find_from_path(u"info/link") != nullptr)
    {
        auto link = dynamic_cast<wz::Property<wz::wzstring> *>(node->find_from_path(u"info/link"))->get();
        node = Wz::Npc->get_root()->find_from_path(link + u".img");
    }
    for (auto &[key, val] : node->get_children())
    {
        if (key == u"stand" || key == u"move" || key == u"say" || key == u"hand" ||
            key == u"eye" || key == u"arm" || key == u"angry" ||
            key == u"eyebrows" || key == u"nose" ||
            key == u"hair" || key == u"smile" || key == u"alert" || key == u"blink")
        {
            npc.a[key] = AnimatedSprite(val[0]);
        }
    }
    npc.index = npc.a.begin()->first;

    node = Wz::String->get_root()->find_from_path(u"Npc.img/" + npc.id.substr(npc.id.find_first_not_of(u'0')));
    if (node != nullptr)
    {
        auto nametag = &World::registry->emplace<NameTag>(ent);
        if (node->get_child(u"name"))
        {
            auto name = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"name"))->get();
            NameTag::push(nametag, name, SDL_Color{255, 205, 0, 255});
        }
        if (node->get_child(u"func"))
        {
            auto func = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"func"))->get();
            NameTag::push(nametag, func, SDL_Color{255, 205, 0, 255});
        }
        std::vector<std::u16string> strs;
        if (node->get_child(u"n0"))
        {
            strs.push_back(dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"n0"))->get());
        }
        if (!strs.empty())
        {
            auto chatballoon = &World::registry->emplace<ChatBalloon>(ent);
            for (auto &str : strs)
            {
                ChatBalloon::push(chatballoon, str, SDL_Color{156, 91, 97, 255});
            }
        }
    }
    World::registry->emplace<Transform>(ent, (float)x, (float)y, layer * LAYER_Z + NPC_Z);
}