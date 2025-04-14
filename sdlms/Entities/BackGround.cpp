#include "BackGround.h"

#include "wz/Property.hpp"
#include "entt/entt.hpp"
#include <SDL3/SDL.h>
#include <variant>

#include "Core/Core.h"
#include "Components/Components.h"
#include "Commons/Commons.h"
#include "Resources/Wz.h"

void load_background(wz::Node *node, int id)
{
    auto bS = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"bS"))->get();
    if (bS == u"")
    {
        return;
    }

    auto ent = World::registry->create();

    auto &bspr = World::registry->emplace<BackGround>(ent);
    auto &tr = World::registry->emplace<Transform>(ent);

    tr.position.x = dynamic_cast<wz::Property<int> *>(node->get_child(u"x"))->get();
    tr.position.y = dynamic_cast<wz::Property<int> *>(node->get_child(u"y"))->get();

    bspr.cx = dynamic_cast<wz::Property<int> *>(node->get_child(u"cx"))->get();
    bspr.cy = dynamic_cast<wz::Property<int> *>(node->get_child(u"cy"))->get();

    bspr.rx = dynamic_cast<wz::Property<int> *>(node->get_child(u"rx"))->get();
    bspr.ry = dynamic_cast<wz::Property<int> *>(node->get_child(u"ry"))->get();

    auto front = 0;
    if (node->get_child(u"front") != nullptr)
    {
        front = dynamic_cast<wz::Property<int> *>(node->get_child(u"front"))->get();
    }

    auto flip = 0;
    if (node->get_child(u"f") != nullptr)
    {
        flip = dynamic_cast<wz::Property<int> *>(node->get_child(u"f"))->get();
    }
    tr.flip = flip;

    auto a = 255;
    if (node->get_child(u"a") != nullptr)
    {
        a = dynamic_cast<wz::Property<int> *>(node->get_child(u"a"))->get();
    }

    if (front == 1)
    {
        // 前景
        tr.z = FRONT_BACKGROUND_Z + id;
    }
    else
    {
        // 背景
        tr.z = id + BACKGROUND_Z;
    }

    auto type = dynamic_cast<wz::Property<int> *>(node->get_child(u"type"))->get();

    switch (type)
    {
    case NORMAL:
        bspr.htile = false;
        bspr.vtile = false;
        break;
    case HTILED:
    case HMOVEA:
        bspr.htile = true;
        bspr.vtile = false;
        break;
    case VTILED:
    case VMOVEA:
        bspr.htile = false;
        bspr.vtile = true;
        break;
    case TILED:
    case HMOVEB:
    case VMOVEB:
        bspr.htile = true;
        bspr.vtile = true;
        break;
    }
    switch (type)
    {
    case HMOVEA:
    case HMOVEB:
        bspr.hspeed = true;
        bspr.vspeed = false;
        break;
    case VMOVEA:
    case VMOVEB:
        bspr.hspeed = false;
        bspr.vspeed = true;
        break;
    default:
        bspr.hspeed = false;
        bspr.vspeed = false;
        break;
    }

    auto ani = 0;
    if (node->get_child(u"ani") != nullptr)
    {
        ani = dynamic_cast<wz::Property<int> *>(node->get_child(u"ani"))->get();
    }

    auto no = dynamic_cast<wz::Property<int> *>(node->get_child(u"no"))->get();
    auto no_str = std::to_string(no);

    switch (ani)
    {
    case 0:
    {
        auto url = u"Back/" + bS + u".img/" + u"back" + u"/" + std::u16string{no_str.begin(), no_str.end()};
        auto spr = Sprite::Wrap::load(Wz::Map->get_root()->find_from_path(url), a);
        bspr.spr = Sprite(spr);
        SDL_SetTextureScaleMode(spr->texture, SDL_SCALEMODE_NEAREST);
        break;
    }
    case 1:
    {
        auto url = u"Back/" + bS + u".img/" + u"ani" + u"/" + std::u16string{no_str.begin(), no_str.end()};
        auto asprw = AnimatedSprite::Wrap::load(Wz::Map->get_root()->find_from_path(url), a);
        bspr.spr = AnimatedSprite(asprw);
        break;
    }
    default:
        break;
    }
}