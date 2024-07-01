#include "Npc.h"
#include "FootHold.h"
#include "NameTag.h"
#include "String.h"
#include "ChatBalloon.h"
#include "Timer.h"

#include "Components/Sprite.h"
#include "Components/Transform.h"
#include "Components/RelativeTransform.h"
#include "Components/RandomInput.h"
#include "Components/LimitTransform.h"
#include "Components/Line.h"
#include "Components/Physic/Normal.h"
#include "Resource/Wz.h"

Npc::Npc(wz::Node *node, int id, int rx0, int rx1, World *world)
{
    if (node != nullptr)
    {
        auto npc_id = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"id"))->get();
        auto x = dynamic_cast<wz::Property<int> *>(node->get_child(u"x"))->get();
        auto fh = dynamic_cast<wz::Property<int> *>(node->get_child(u"fh"))->get();
        auto foo = world->get_entitys<FootHold>().find(fh)->second;
        // 从fh获取y,layer
        auto y = foo->get_component<Line>()->get_y(x).value();
        auto layer = foo->get_page();

        node = world->get_resource<Wz>().Npc->get_root()->find_from_path(npc_id + u".img");
        // 排除 link
        while (node->find_from_path(u"info/link") != nullptr)
        {
            auto link = dynamic_cast<wz::Property<wz::wzstring> *>(node->find_from_path(u"info/link"))->get();
            node = world->get_resource<Wz>().Npc->get_root()->find_from_path(link + u".img");
        }
        AnimatedSprite *aspr = nullptr;
        for (auto &[name, val] : node->get_children())
        {
            if (name != u"info")
            {
                aspr = new AnimatedSprite(val[0]);
                aspr_map[name] = aspr;
                world->add_component(aspr);
            }
        }
        Transform *tr = new Transform{(float)x, (float)y};
        add_component(tr);
        world->add_component(tr, layer * 300000 + id + 20000);

        if (aspr_map.size() > 0)
        {
            // 默认显示npc第一个状态
            act = aspr_map.begin()->first;
            auto aspr = aspr_map.begin()->second;
            add_component(aspr);
        }

        // 添加物理组件
        if (aspr_map.contains(u"move"))
        {
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
        }
        // 从string.wz获取信息
        node = world->get_resource<Wz>().String->get_root()->find_from_path(u"Npc.img/" + npc_id.substr(npc_id.find_first_not_of(u'0')));
        if (node != nullptr)
        {
            int i = 0;
            for (auto &[key, val] : node->get_children())
            {
                str_map[key] = dynamic_cast<wz::Property<wz::wzstring> *>(val[0])->get();
                if (key == u"name" || key == u"func")
                {
                    auto str = new String(str_map[key], {255, 255, 0, 255});
                    add_entity(str);
                    auto spr = str->get_component<Sprite>();
                    {
                        auto nam = new NameTag(spr->width, spr->height);
                        add_entity(nam);
                        auto rtr = new RelativeTransform(tr, SDL_FPoint{(float)(-spr->get_width() / 2), (float)(6 + (spr->get_height() + 6) * i)});
                        nam->add_component(rtr);
                        nam->add_component(new Transform());
                        world->add_component(rtr, 0);
                    }
                    {
                        auto rtr = new RelativeTransform(tr, SDL_FPoint{(float)(-spr->get_width() / 2 + 2), (float)(9 + (spr->get_height() + 6) * i)});
                        str->add_component(rtr);
                        str->add_component(new Transform());
                        world->add_component(rtr, 0);
                    }
                    i++;
                }
                else if (get_entity<ChatBalloon>(0) == nullptr)
                {
                    auto str = new String(str_map[key], {88, 0, 0, 255}, 8, 13);
                    add_entity(str);

                    // 添加聊天气泡
                    auto bal = new ChatBalloon(str->get_width(), str->get_height(), u"0");
                    bal->add_entity(str);
                    add_entity(bal);
                    auto rtr = new RelativeTransform(tr, SDL_FPoint{(float)(-bal->get_width() / 2), (float)-aspr->get_anim_height() - (aspr->get_current_sprite()->origin.y - aspr->get_anim_height()) - (float)bal->get_height()});
                    bal->add_component(rtr);
                    bal->add_component(new Transform());
                    world->add_component(rtr, 0);

                    auto callback = [](Uint32 interval, void *param) -> Uint32
                    {
                        auto world = World::get_world();
                        auto timer = (Timer *)param;
                        auto bal = timer->get_entity<ChatBalloon>(0);
                        {
                            auto spr = bal->get_component<Sprite>();
                            if (spr == nullptr)
                            {
                                bal->add_component(bal->spr);
                            }
                            else
                            {
                                bal->remove_component<Sprite>();
                            }
                        }
                        auto str = bal->get_entity<String>(0);
                        {
                            auto spr = str->get_component<Sprite>();
                            if (spr == nullptr)
                            {
                                str->add_component(str->spr);
                            }
                            else
                            {
                                str->remove_component<Sprite>();
                            }
                        }
                        return 2000;
                    };
                    Timer *timer = new Timer(callback, 2000);
                    add_entity(timer);
                    world->add_entity(timer);
                    timer->add_entity(bal, 0);

                    rtr = new RelativeTransform(tr, SDL_FPoint{(float)(-bal->get_width() / 2 + 8), (float)-aspr->get_anim_height() - (aspr->get_current_sprite()->origin.y - aspr->get_anim_height()) - (float)bal->get_height() + 6});
                    str->add_component(rtr);
                    str->add_component(new Transform());
                    world->add_component(rtr, 0);
                }
            }
        }
    }
}

void Npc::switch_act(const std::u16string &action)
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

void Npc::start_animate()
{
    get_component<AnimatedSprite>()->set_animate(true);
}

void Npc::stop_animate()
{
    get_component<AnimatedSprite>()->set_animate(false);
}

Npc::~Npc()
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

    for (auto &[key, val] : get_entity<ChatBalloon>())
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

    for (auto &[key, val] : get_entity<Timer>())
    {
        world->remove_entity(val);
        delete val;
    }
}
