#include "Npc.h"
#include "FootHold.h"

#include "Components/Sprite.h"
#include "Components/Transform.h"
#include "Resource/Wz.h"
#include "Core/FreeType.h"

Npc::Npc(wz::Node *node, World *world)
{
    if (node != nullptr)
    {
        auto npc_id = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"id"))->get();
        auto x = dynamic_cast<wz::Property<int> *>(node->get_child(u"x"))->get();
        auto y = dynamic_cast<wz::Property<int> *>(node->get_child(u"cy"))->get();
        auto fh = dynamic_cast<wz::Property<int> *>(node->get_child(u"fh"))->get();
        // 从fh获取layer
        auto layer = world->get_entitys<FootHold>().find(fh)->second->get_page();
        auto xm = dynamic_cast<wz::Property<int> *>(node->get_child(u"x"))->get();

        node = world->get_resource<Wz>().Npc->get_root()->find_from_path(npc_id + u".img");
        for (auto &[name, val] : node->get_children())
        {
            if (name != u"info")
            {
                auto aspr = AnimatedSprite::load_animated_sprite(val[0]);
                aspr_map[name] = aspr;
            }
        }

        Transform *t = new Transform{(float)x, (float)y};
        auto aspr = aspr_map.begin()->second;
        add_component(t);
        add_component(aspr);
        world->add_component(t, 30000 * layer + 3000);
        world->add_unique_component(aspr);

        // 从string.wz获取信息
        node = world->get_resource<Wz>().String->get_root()->find_from_path(u"Npc.img/" + npc_id.substr(npc_id.find_first_not_of(u'0')));
        if (node != nullptr)
        {
            for (auto &[key, val] : node->get_children())
            {
                str_map[key] = dynamic_cast<wz::Property<wz::wzstring> *>(val[0])->get();
            }
            if (str_map.contains(u"name"))
            {
                name = FreeType::str(str_map[u"name"]);
            }
            if (str_map.contains(u"func"))
            {
                func = FreeType::str(str_map[u"func"]);
            }
        }
    }
}