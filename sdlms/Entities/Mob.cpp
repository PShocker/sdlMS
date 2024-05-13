#include "Mob.h"

#include "FootHold.h"

#include "Components/Sprite.h"
#include "Components/Transform.h"
#include "Resource/Wz.h"
#include "Core/FreeType.h"

Mob::Mob(wz::Node *node, World *world)
{
    if (node != nullptr)
    {
        auto mob_id = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"id"))->get();
        auto x = dynamic_cast<wz::Property<int> *>(node->get_child(u"x"))->get();
        auto y = dynamic_cast<wz::Property<int> *>(node->get_child(u"cy"))->get();
        auto fh = dynamic_cast<wz::Property<int> *>(node->get_child(u"fh"))->get();
        // 从fh获取layer
        auto layer = world->get_entitys<FootHold>().find(fh)->second->get_page();

        node = world->get_resource<Wz>().Mob->get_root()->find_from_path(mob_id + u".img");
        // 排除 link
        while (node->find_from_path(u"info/link") != nullptr)
        {
            auto link = dynamic_cast<wz::Property<wz::wzstring> *>(node->find_from_path(u"info/link"))->get();
            node = world->get_resource<Wz>().Mob->get_root()->find_from_path(link + u".img");
        }
        for (auto &[name, val] : node->get_children())
        {
            if (name != u"info")
            {
                auto aspr = new AnimatedSprite(val[0]);
                aspr_map[name] = aspr;
            }
        }
        if (aspr_map.size() > 0)
        {
            // 默认显示npc第一个状态
            Transform *t = new Transform{(float)x, (float)y};
            auto aspr = aspr_map.begin()->second;
            add_component(t);
            add_component(aspr);
            world->add_component(t, 30000 * layer + 3000);
            world->add_component(aspr);
        }
        // 从string.wz获取信息
        node = world->get_resource<Wz>().String->get_root()->find_from_path(u"Mob.img/" + mob_id.substr(mob_id.find_first_not_of(u'0')));
        if (node != nullptr)
        {
            if (node->get_child(u"name") != nullptr)
            {
                name = FreeType::str(dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"name"))->get());
            }
        }
    }
}

// 测试
Mob::Mob(World *world)
{
    std::u16string mob_id = u"1210102";
    auto node = world->get_resource<Wz>().Mob->get_root()->find_from_path(mob_id + u".img");
    // 排除 link
    while (node->find_from_path(u"info/link") != nullptr)
    {
        auto link = dynamic_cast<wz::Property<wz::wzstring> *>(node->find_from_path(u"info/link"))->get();
        node = world->get_resource<Wz>().Mob->get_root()->find_from_path(link + u".img");
    }
    for (auto &[name, val] : node->get_children())
    {
        if (name != u"info")
        {
            auto aspr = new AnimatedSprite(val[0]);
            aspr_map[name] = aspr;
            world->add_component(aspr);
        }
    }
    act = u"jump";
    add_component(aspr_map[act]);

    // 从string.wz获取信息
    node = world->get_resource<Wz>().String->get_root()->find_from_path(u"Mob.img/" + mob_id.substr(mob_id.find_first_not_of(u'0')));
    if (node != nullptr)
    {
        if (node->get_child(u"name") != nullptr)
        {
            name = FreeType::str(dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"name"))->get());
        }
    }
}

void Mob::switch_act(const std::u16string &a)
{
    if (a != act)
    {
        if (aspr_map.contains(a))
        {
            auto aspr = aspr_map[a];
            aspr->set_anim_index(0);
            aspr->set_anim_time(0);
            act = a;
            add_component(aspr);
        }
    }
}

Mob::~Mob()
{
    auto world = World::get_world();
    
    for (auto &[key, val] : aspr_map)
    {
        auto aspr = val;
        world->destroy_component(aspr, false);
        delete aspr;
    }
    if (name != nullptr)
    {
        delete name;
    }
    auto t = get_component<Transform>();
    world->destroy_component(t, false);
    delete t;
}