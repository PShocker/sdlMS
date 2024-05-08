#include "Portal.h"
#include "Resource/Wz.h"
#include "Components/Sprite.h"
#include "Components/Transform.h"

Portal::Portal(wz::Node *node, World *world)
{
    constexpr const char16_t *pt_list[] = {u"sp", u"pi", u"pv", u"pc", u"pg", u"tp", u"ps",
                                           u"pgi", u"psi", u"pcs", u"ph", u"psh", u"pcj",
                                           u"pci", u"pcig", u"pshg"};
    if (node->get_child(u"pt") != nullptr)
    {

        auto pt = dynamic_cast<wz::Property<int> *>(node->get_child(u"pt"))->get();
        auto tm = dynamic_cast<wz::Property<int> *>(node->get_child(u"tm"))->get();

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
                // Sprite *spr = Sprite::load_sprite(pn);
                // Transform *t = new Transform{(float)x, (float)y};
                // add_component(t);
                // add_component(spr);
                // world->add_component(t, 99999999);
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
                        {
                            auto aspr = AnimatedSprite::load_animated_sprite(world->get_resource<Wz>().Map->get_root()->find_from_path(url + u"/default/portalStart"));
                            aspr_map[u"portalStart"] = aspr;
                            world->add_unique_component(aspr);
                        }
                        {
                            auto aspr = AnimatedSprite::load_animated_sprite(world->get_resource<Wz>().Map->get_root()->find_from_path(url + u"/default/portalContinue"));
                            aspr_map[u"portalContinue"] = aspr;
                            world->add_unique_component(aspr);
                        }
                        {
                            auto aspr = AnimatedSprite::load_animated_sprite(world->get_resource<Wz>().Map->get_root()->find_from_path(url + u"/default/portalExit"));
                            aspr_map[u"portalExit"] = aspr;
                            world->add_unique_component(aspr);
                        }
                        Transform *t = new Transform{(float)x, (float)y};
                        add_component(t);
                        world->add_component(t, 99999999);
                    }
                    else
                    {
                        // 普通的传送门,通常为pv
                        auto aspr = AnimatedSprite::load_animated_sprite(world->get_resource<Wz>().Map->get_root()->find_from_path(url));
                        Transform *t = new Transform{(float)x, (float)y};
                        add_component(t);
                        add_component(aspr);
                        world->add_component(t, 99999999);
                        world->add_unique_component(aspr);
                    }
                }
            }
        }
    }
}
