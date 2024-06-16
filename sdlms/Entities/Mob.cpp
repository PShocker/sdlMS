#include "Mob.h"

#include "FootHold.h"
#include "NameTag.h"
#include "String.h"

#include "Components/Sprite.h"
#include "Components/Transform.h"
#include "Components/RelativeTransform.h"
#include "Components/RandomInput.h"
#include "Components/LimitTransform.h"
#include "Components/Line.h"
#include "Components/Physic/Normal.h"
#include "Resource/Wz.h"

Mob::Mob(wz::Node *node, int id, int rx0, int rx1, World *world)
{
    if (node != nullptr)
    {
        auto mob_id = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"id"))->get();
        auto x = dynamic_cast<wz::Property<int> *>(node->get_child(u"x"))->get();
        auto fh = dynamic_cast<wz::Property<int> *>(node->get_child(u"fh"))->get();
        auto foo = world->get_entitys<FootHold>().find(fh)->second;
        // 从fh获取y,layer
        auto y = foo->get_component<Line>()->get_y(x).value();
        auto layer = foo->get_page();

        node = world->get_resource<Wz>().Mob->get_root()->find_from_path(mob_id + u".img");
        // 排除 link
        while (node->find_from_path(u"info/link") != nullptr)
        {
            auto link = dynamic_cast<wz::Property<wz::wzstring> *>(node->find_from_path(u"info/link"))->get();
            node = world->get_resource<Wz>().Npc->get_root()->find_from_path(link + u".img");
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
        Transform *tr = new Transform((float)x, (float)y);
        add_component(tr);
        world->add_component(tr, 30000 * layer + 3000 + id);

        if (aspr_map.size() > 0)
        {
            // 默认显示mob第一个状态
            auto aspr = aspr_map.begin()->second;
            add_component(aspr);
        }
        // 添加物理组件
        Normal *nor = new Normal();
        nor->type = Normal::Ground;
        add_entity(foo, 0);
        add_component(nor);
        world->add_component(nor);

        // 添加自动移动组件
        RandomInput *ran = new RandomInput();
        add_component(ran);
        world->add_component(ran);

        // 添加移动范围
        LimitTransform *ltr = new LimitTransform(tr, SDL_FPoint{(float)rx0, (float)rx1}, std::nullopt);
        add_component(ltr);
        world->add_component(ltr);

        // 从string.wz获取信息
        node = world->get_resource<Wz>().String->get_root()->find_from_path(u"Mob.img/" + mob_id.substr(mob_id.find_first_not_of(u'0')));
        if (node != nullptr)
        {
            auto name = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"name"))->get();
            auto str = new String(name, {255, 255, 255, 255}, 12);
            add_entity(str);
            auto spr = str->get_component<Sprite>();
            {
                auto rtr = new RelativeTransform(tr, SDL_FPoint{(float)(-spr->get_width() / 2 + 2), 9});
                str->add_component(rtr);
                str->add_component(new Transform());
                world->add_component(rtr, 1);
            }
            {
                auto nam = new NameTag(spr->width + 4, spr->height + 6);
                add_entity(nam);
                auto rtr = new RelativeTransform(tr, SDL_FPoint{(float)(-spr->get_width() / 2), 6});
                nam->add_component(rtr);
                nam->add_component(new Transform());
                world->add_component(rtr, 0);
            }
        }
    }
}

// 测试
Mob::Mob(World *world, SDL_FPoint p)
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

    Transform *tr = new Transform(p);
    add_component(tr);
    world->add_component(tr, 3000000);
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

    for (auto &[key, val] : get_entity<String>())
    {
        auto t = val->get_component<Transform>();
        auto rtr = val->get_component<RelativeTransform>();
        world->destroy_component(t, false);
        world->destroy_component(rtr, false);
        delete t;
        delete rtr;
        delete val;
    }

    for (auto &[key, val] : get_entity<NameTag>())
    {
        auto t = val->get_component<Transform>();
        auto rtr = val->get_component<RelativeTransform>();
        world->destroy_component(t, false);
        world->destroy_component(rtr, false);
        delete t;
        delete rtr;
        delete val;
    }

    auto t = get_component<Transform>();
    world->destroy_component(t, false);
    delete t;

    auto nor = get_component<Normal>();
    if (nor != nullptr)
    {
        world->destroy_component(nor, false);
        delete nor;
    }

    auto ran = get_component<RandomInput>();
    if (ran != nullptr)
    {
        world->destroy_component(ran, false);
        delete ran;
    }

    auto ltr = get_component<LimitTransform>();
    if (ltr != nullptr)
    {
        world->destroy_component(ltr, false);
        delete ltr;
    }
}
