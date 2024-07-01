#include "Portal.h"
#include "Resource/Wz.h"
#include "Components/Sprite.h"
#include "Components/Transform.h"
#include "Components/DistanceSprite.h"
#include "Timer.h"

Portal::Portal(wz::Node *node, World *world)
{
    constexpr const char16_t *pt_list[] = {u"sp", u"pi", u"pv", u"pc", u"pg", u"tp", u"ps",
                                           u"pgi", u"psi", u"pcs", u"ph", u"psh", u"pcj",
                                           u"pci", u"pcig", u"pshg"};
    if (node->get_child(u"pt") != nullptr)
    {

        pt = dynamic_cast<wz::Property<int> *>(node->get_child(u"pt"))->get();
        tm = dynamic_cast<wz::Property<int> *>(node->get_child(u"tm"))->get();
        pn = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"pn"))->get();
        tn = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"tn"))->get();

        if (pt < 0 || pt >= sizeof(pt_list))
        {
            return;
        }
        else
        {
            auto x = dynamic_cast<wz::Property<int> *>(node->get_child(u"x"))->get();
            auto y = dynamic_cast<wz::Property<int> *>(node->get_child(u"y"))->get();
            {
                
                // auto url = u"MapHelper.img/portal/editor/" + std::basic_string<char16_t>(pt_list[pt]);
                // auto pn = world->get_resource<Wz>().Map->get_root()->find_from_path(url);
                // Sprite *spr = new Sprite(pn);
                // add_component(spr);
                Transform *t = new Transform{(float)x, (float)y};
                add_component(t);
                world->add_component(t, 99999999);
            }

            {
                if (pt == 7)
                {
                    pt = 2;
                }
                auto url = u"MapHelper.img/portal/game/" + std::basic_string<char16_t>(pt_list[pt]);
                if (world->get_resource<Wz>().Map->get_root()->find_from_path(url) != NULL)
                {
                    if (world->get_resource<Wz>().Map->get_root()->find_from_path(url + u"/default") != NULL)
                    {
                        auto disspr = new DistanceSprite();
                        add_component(disspr);
                        world->add_component(disspr);
                        {
                            auto aspr = new AnimatedSprite(world->get_resource<Wz>().Map->get_root()->find_from_path(url + u"/default/portalContinue"));
                            disspr->add(SDL_FPoint{-100, 100}, SDL_FPoint{-50, 50}, aspr);
                            aspr_map[u"portalContinue"] = aspr;
                            world->add_component(aspr);
                        }
                        {
                            auto aspr = new AnimatedSprite(world->get_resource<Wz>().Map->get_root()->find_from_path(url + u"/default/portalExit"));
                            disspr->add(SDL_FPoint{-150, 150}, SDL_FPoint{-100, 100}, aspr);
                            aspr_map[u"portalExit"] = aspr;
                            world->add_component(aspr);
                        }
                        {
                            auto aspr = new AnimatedSprite(world->get_resource<Wz>().Map->get_root()->find_from_path(url + u"/default/portalStart"));
                            disspr->add(std::nullopt, std::nullopt, std::nullopt);
                            aspr_map[u"portalStart"] = aspr;
                            world->add_component(aspr);
                        }
                    }
                    else
                    {
                        // 普通的传送门,通常为pv
                        auto aspr = new AnimatedSprite(world->get_resource<Wz>().Map->get_root()->find_from_path(url));
                        add_component(aspr);
                        world->add_component(aspr);
                    }
                }
            }
        }
    }
}

Portal::~Portal()
{
    auto world = World::get_world();

    if (aspr_map.size() > 0)
    {
        for (auto &[key, val] : aspr_map)
        {
            auto aspr = val;
            world->destroy_component(aspr, false);
            delete aspr;
        }
    }
    else if (get_component<AnimatedSprite>() != nullptr)
    {
        auto aspr = get_component<AnimatedSprite>();
        if (aspr != nullptr)
        {
            world->destroy_component(aspr, false);
            delete aspr;
        }
    }

    auto spr = get_component<Sprite>();
    if (spr != nullptr)
    {
        delete spr;
    }

    auto t = get_component<Transform>();
    if (t != nullptr)
    {
        world->destroy_component(t, false);
        delete t;
    }

    auto disspr = get_component<DistanceSprite>();
    if (disspr != nullptr)
    {
        world->destroy_component(disspr, false);
        delete disspr;
    }

    for (auto &[key, val] : get_entity<Timer>())
    {
        world->remove_entity(val);
        delete val;
    }
}
