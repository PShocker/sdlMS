#include "Portal.h"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include <string>
#include <variant>
#include "Core/Core.h"
#include "Components/Components.h"
#include "Commons/Commons.h"
#include "Resources/Wz.h"

const std::u16string pt_list[] = {u"sp", u"pi", u"pv", u"pc", u"pg", u"tp", u"ps",
                                  u"pgi", u"psi", u"pcs", u"ph", u"psh", u"pcj",
                                  u"pci", u"pcig", u"pshg"};

void load_portal(wz::Node *node, int id)
{
    auto pt = dynamic_cast<wz::Property<int> *>(node->get_child(u"pt"))->get();

    if (!(pt < 0 || pt >= sizeof(pt_list)))
    {
        auto ent = World::registry->create();

        auto tm = dynamic_cast<wz::Property<int> *>(node->get_child(u"tm"))->get();
        auto pn = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"pn"))->get();
        auto tn = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"tn"))->get();

        auto x = dynamic_cast<wz::Property<int> *>(node->get_child(u"x"))->get();
        auto y = dynamic_cast<wz::Property<int> *>(node->get_child(u"y"))->get();

        if (pt == 7)
        {
            pt = 2;
        }

        auto &por = World::registry->emplace<Portal>(ent);
        por.tm = tm;
        por.pt = pt;
        por.pn = pn;
        por.tn = tn;

        World::registry->emplace<Transform>(ent, x, y, id + PORTAL_Z);

        auto url = u"MapHelper.img/portal/game/" + pt_list[pt];
        if (auto portal = Wz::Map->get_root()->find_from_path(url))
        {
            if (portal->get_child(u"default") != nullptr)
            {
                // 三段式传送门
                auto asprw = AnimatedSprite::Wrap::load(portal->find_from_path(u"default/portalContinue"));
                por.a.push_back(AnimatedSprite(asprw));

                asprw = AnimatedSprite::Wrap::load(portal->find_from_path(u"default/portalExit"));
                por.a.push_back(AnimatedSprite(asprw));

                asprw = AnimatedSprite::Wrap::load(portal->find_from_path(u"default/portalStart"));
                por.a.push_back(AnimatedSprite(asprw));
            }
            else
            {
                // 普通的传送门,通常为pv
                auto asprw = AnimatedSprite::Wrap::load(portal);
                por.a.push_back(AnimatedSprite(asprw));
                if (Map::load_string_node(tm))
                {
                    auto nametag = &World::registry->emplace<NameTag>(ent);
                    nametag->offset_y = 4;
                    NameTag::push(nametag, Map::load_mapname(tm), SDL_Color{255, 205, 0, 255});
                }
            }
        }
    }
}

void fix_portal()
{
    std::unordered_map<std::u16string, SDL_FPoint> pors;

    auto view = World::registry->view<Portal>();
    for (auto &e : view)
    {
        auto por = &view.get<Portal>(e);
        auto tr = World::registry->try_get<Transform>(e);
        pors[por->pn] = SDL_FPoint{tr->position.x, tr->position.y};
    }
    for (auto &e : view)
    {
        auto por = &view.get<Portal>(e);
        if (por->tm == Map::id)
        {
            auto por = &view.get<Portal>(e);
            por->tn = pors[std::get<std::u16string>(por->tn)];
        }
    }
}

SDL_FPoint recent_portal(float x, float y)
{
    float min_sq_dist = std::numeric_limits<float>::max();
    auto view = World::registry->view<Portal>();
    for (auto &e : view)
    {
        auto por = &view.get<Portal>(e);
        if (por->pn == u"sp")
        {
            auto tr = World::registry->try_get<Transform>(e);
            // 计算平方距离
            const float sq_dist = squared_distance(tr->position, SDL_FPoint{x, y});
            if (sq_dist < min_sq_dist)
            {
                min_sq_dist = sq_dist;
                x = tr->position.x;
                y = tr->position.y;
            }
        }
    }
    return SDL_FPoint{x, y};
}
