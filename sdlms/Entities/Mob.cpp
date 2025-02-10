#include "Mob.h"
#include "wz/Property.hpp"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include "Core/Core.h"
#include "Components/Components.h"
#include "Commons/Commons.h"
#include "Resources/Wz.h"
#include "FootHold.h"

void load_mob(wz::Node *node)
{
    auto ent = World::registry->create();

    auto fh = dynamic_cast<wz::Property<int> *>(node->get_child(u"fh"))->get();
    auto foo = fhs[fh];

    auto &mv = World::registry->emplace<Move>(ent);
    mv.rx0 = dynamic_cast<wz::Property<int> *>(node->get_child(u"rx0"))->get();
    mv.rx1 = dynamic_cast<wz::Property<int> *>(node->get_child(u"rx1"))->get();
    mv.foo = foo;

    auto &mob = World::registry->emplace<Mob>(ent);
    auto id = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"id"))->get();

    auto x = dynamic_cast<wz::Property<int> *>(node->get_child(u"x"))->get();
    // 从fh获取y,layer
    auto y = foo->get_y(x).value();
    auto layer = foo->page;

    mob.id = id;
    mob.init_x = x;
    mob.init_y = y;
    mob.init_fh = foo;

    node = Wz::Mob->get_root()->find_from_path(id + u".img");
    // 排除 link
    while (node->find_from_path(u"info/link") != nullptr)
    {
        auto link = dynamic_cast<wz::Property<wz::wzstring> *>(node->find_from_path(u"info/link"))->get();
        node = Wz::Mob->get_root()->find_from_path(link + u".img");
    }
    for (auto &[key, val] : node->get_children())
    {
        if (key != u"info")
        {
            auto asprw = AnimatedSpriteWarp::load(val[0]);
            mob.a[key] = AnimatedSprite(asprw);

            for (auto &sprs : mob.a[key].aspr->sprites)
            {
                SDL_SetTextureScaleMode(sprs->texture, SDL_SCALEMODE_NEAREST);
            }
            if (key == u"attack1")
            {
                asprw = AnimatedSpriteWarp::load(val[0]->find_from_path(u"info/hit"));
                mob.atk = asprw;
                mob.atkw.hit = mob.atk;
            }
        }
        else
        {
            // key = info
            auto speed = 0;
            if (val[0]->get_child(u"speed") != nullptr)
            {
                speed = dynamic_cast<wz::Property<int> *>(val[0]->get_child(u"speed"))->get();
            }
            mv.hspeed_min = -1 * (float)(speed + 100) / 100 * 125;
            mv.hspeed_max = (float)(speed + 100) / 100 * 125;
            mv.hspeed = mv.hspeed_max.value();
        }
    }
    if (mob.a.contains(u"stand"))
    {
        mob.index = u"stand";
    }
    else if (mob.a.contains(u"fly"))
    {
        mob.index = u"fly";
        mob.state = Mob::State::FLY;
    }

    node = Wz::Sound->get_root()->find_from_path(u"Mob.img/" + id);
    if (node != nullptr)
    {
        for (auto &[key, val] : node->get_children())
        {
            auto sou = SoundWarp::load(val[0]);
            mob.sounds[key] = sou;
        }
    }
    World::registry->emplace<Hit>(ent);
    World::registry->emplace<Effect>(ent);
    World::registry->emplace<Damage>(ent);
    World::registry->emplace<Animated>(ent);
    World::registry->emplace<Transform>(ent, (float)x, (float)y, layer * LAYER_Z + MOB_Z);
}