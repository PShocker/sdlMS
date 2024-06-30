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
            node = world->get_resource<Wz>().Mob->get_root()->find_from_path(link + u".img");
        }

        Transform *tr = new Transform((float)x, (float)y);
        add_component(tr);
        world->add_component(tr, layer * 300000 + id + 30000);

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

        for (auto &[name, val] : node->get_children())
        {
            if (name == u"info")
            {
                // 获取mob基础属性
                auto speed = 0;
                if (val[0]->get_child(u"speed") != nullptr)
                {
                    speed = dynamic_cast<wz::Property<int> *>(val[0]->get_child(u"speed"))->get();
                }
                nor->hspeed_limit = {-1 * (float)(speed + 100) / 100 * 125, (float)(speed + 100) / 100 * 125};
            }
            else
            {
                auto aspr = new AnimatedSprite(val[0]);
                aspr_map[name] = aspr;
                world->add_component(aspr);
            }
        }
        if (aspr_map.size() > 0)
        {
            // 默认显示mob第一个状态
            auto aspr = aspr_map.begin()->second;
            add_component(aspr);
        }

        // 从string.wz获取信息
        node = world->get_resource<Wz>().String->get_root()->find_from_path(u"Mob.img/" + mob_id.substr(mob_id.find_first_not_of(u'0')));
        if (node != nullptr)
        {
            auto name = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"name"))->get();
            auto str = new String(name, {255, 255, 255, 255}, 0, 12);
            add_entity(str);
            auto spr = str->get_component<Sprite>();

            auto nam = new NameTag(spr->width, spr->height);
            add_entity(nam);
            auto rtr = new RelativeTransform(tr, SDL_FPoint{(float)(-nam->get_width() / 2), 6});
            nam->add_component(rtr);
            nam->add_component(new Transform());
            world->add_component(rtr, 0);

            rtr = new RelativeTransform(tr, SDL_FPoint{(float)(-spr->get_width() / 2), (float)6 + (nam->get_height() - spr->height) / 2});
            str->add_component(rtr);
            str->add_component(new Transform());
            world->add_component(rtr, 0);
        }
    }
}

// 测试
Mob::Mob(World *world, SDL_FPoint p)
{
    const std::u16string mob_id = u"1210102";
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

void Mob::switch_act(const std::u16string &action)
{
    if (action != act)
    {
        if (aspr_map.contains(action))
        {
            auto aspr = aspr_map[action];
            aspr->set_anim_index(0);
            aspr->set_anim_time(0);
            act = action;
            add_component(aspr);
        }
    }
}

void Mob::start_animate()
{
    get_component<AnimatedSprite>()->set_animate(true);
}

void Mob::stop_animate()
{
    get_component<AnimatedSprite>()->set_animate(false);
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
