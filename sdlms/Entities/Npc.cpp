#include "Npc.h"
#include "FootHold.h"

#include "Components/Sprite.h"
#include "Components/Transform.h"
#include "Components/AnimatedSprite.h"
#include "Resource/Wz.h"

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
                Transform *t = new Transform{(float)x, (float)y};
                add_component(t);
                add_component(aspr);
                world->add_component(t, 30000 * layer + 3000);
                world->add_unique_component(aspr);
            }
        }
    }
}