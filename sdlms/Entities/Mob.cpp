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

    auto link = id;

    node = Wz::Mob->get_root()->find_from_path(id + u".img");
    // 排除 link
    while (node->find_from_path(u"info/link") != nullptr)
    {
        link = dynamic_cast<wz::Property<wz::wzstring> *>(node->find_from_path(u"info/link"))->get();
        node = Wz::Mob->get_root()->find_from_path(link + u".img");
    }
    for (auto &[key, val] : node->get_children())
    {
        if (key != u"info")
        {
            mob.a[key] = AnimatedSprite(val[0]);
            for (auto &sprs : mob.a[key].asprw->sprites)
            {
                SDL_SetTextureScaleMode(sprs->texture, SDL_SCALEMODE_NEAREST);
            }
        }
        else
        {
            auto speed = 0;
            if (val[0]->get_child(u"speed") != nullptr)
            {
                speed = dynamic_cast<wz::Property<int> *>(val[0]->get_child(u"speed"))->get();
                mv.hspeed_min = -1 * (float)(speed + 100) / 100 * 125;
                mv.hspeed_max = (float)(speed + 100) / 100 * 125;
            }
            else if (val[0]->get_child(u"flySpeed") != nullptr)
            {
                speed = dynamic_cast<wz::Property<int> *>(val[0]->get_child(u"flySpeed"))->get();
                mv.hspeed_min = -1 * (float)(speed + 100) / 200 * 125;
                mv.hspeed_max = (float)(speed + 100) / 200 * 125;
                mv.ry0 = mob.init_y - 20;
                mv.ry1 = mob.init_y + 20;
            }
        }
    }
    if (node->find_from_path(u"attack1/info/hit"))
    {
        mob.atk.hit = AnimatedSprite::Wrap::load(node->find_from_path(u"attack1/info/hit"));
    }
    if (mob.a.contains(u"move"))
    {
        mob.state = Mob::State::STAND;
        mob.index = u"stand";
    }
    else if (mob.a.contains(u"stand"))
    {
        mob.state = Mob::State::STAY;
        mob.index = u"stand";
    }
    else if (mob.a.contains(u"fly"))
    {
        mob.state = Mob::State::FLY;
        mob.index = u"fly";
    }

    node = Wz::Sound->get_root()->find_from_path(u"Mob.img/" + id);
    if (node != nullptr)
    {
        for (auto &[key, val] : node->get_children())
        {
            auto sou = Sound::Wrap::load(val[0]);
            mob.sounds[key] = sou;
        }
    }
    if (!mob.drops.contains(mob.id))
    {
        std::vector<std::u16string> drops;
        size_t first_non_zero = link.find_first_not_of(u'0');
        // 如果找到了非零字符，则从该位置开始截取字符串
        if (first_non_zero != std::string::npos)
        {
            link = link.substr(first_non_zero);
        }
        node = Wz::String->get_root()->find_from_path(u"MonsterBook.img/" + link + u"/reward");
        if (node != nullptr)
        {
            for (auto &[key, val] : node->get_children())
            {
                std::string str = std::to_string(dynamic_cast<wz::Property<int> *>(val[0])->get());
                auto ustr = std::u16string(str.begin(), str.end());
                Item::load(ustr);
                Item::load_item_string(ustr);
                drops.push_back(ustr);
            }
        }
        mob.drops[mob.id] = drops;
    }

    node = Wz::String->get_root()->find_from_path(u"Mob.img/" + mob.id.substr(mob.id.find_first_not_of(u'0')));
    if (node != nullptr)
    {
        auto name = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"name"))->get();
        auto nametag = &World::registry->emplace<NameTag>(ent);
        NameTag::push(nametag, name, SDL_Color{255, 255, 255, 255});
    }

    World::registry->emplace<Effect>(ent);
    World::registry->emplace<Damage>(ent);
    auto count = World::registry->view<Mob>().size();
    World::registry->emplace<Transform>(ent, (float)x, (float)y, layer * LAYER_Z + MOB_Z + count);
}