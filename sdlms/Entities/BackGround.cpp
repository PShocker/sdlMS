module;

#include "wz/Property.hpp"

module entities;

import components;
import resources;
import core;

void load_background(wz::Node *node, int id)
{
    const int z_index = -10000;

    auto ent = World::registry.create();

    auto bS = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"bS"))->get();
    if (bS == u"")
    {
        return;
    }

    auto ani = 0;
    if (node->get_child(u"ani") != nullptr)
    {
        ani = dynamic_cast<wz::Property<int> *>(node->get_child(u"ani"))->get();
    }

    auto x = dynamic_cast<wz::Property<int> *>(node->get_child(u"x"))->get();
    auto y = dynamic_cast<wz::Property<int> *>(node->get_child(u"y"))->get();

    auto cx = dynamic_cast<wz::Property<int> *>(node->get_child(u"cx"))->get();
    auto cy = dynamic_cast<wz::Property<int> *>(node->get_child(u"cy"))->get();

    auto rx = dynamic_cast<wz::Property<int> *>(node->get_child(u"rx"))->get();
    auto ry = dynamic_cast<wz::Property<int> *>(node->get_child(u"ry"))->get();

    auto type = dynamic_cast<wz::Property<int> *>(node->get_child(u"type"))->get();

    auto no = dynamic_cast<wz::Property<int> *>(node->get_child(u"no"))->get();

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

    auto a = 255;
    if (node->get_child(u"a") != nullptr)
    {
        a = dynamic_cast<wz::Property<int> *>(node->get_child(u"a"))->get();
    }

    auto no_str = std::to_string(no);

    if (front == 1)
    {
        // 前景
        World::registry.emplace<Transform>(ent, x, y, 99999999 + id + z_index);
    }
    else
    {
        // 背景
        World::registry.emplace<Transform>(ent, x, y, id + z_index);
    }

    auto &bspr = World::registry.emplace<BackGround>(ent);
    bspr.cx = cx;
    bspr.cy = cy;
    bspr.rx = rx;
    bspr.ry = ry;

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

    switch (ani)
    {
    case 0:
    {
        auto url = u"Back/" + bS + u".img/" + u"back" + u"/" + std::u16string{no_str.begin(), no_str.end()};
        Sprite *spr = new Sprite(Wz::Map->get_root()->find_from_path(url), a);
        bspr.spr = spr;
        break;
    }
    case 1:
    {
        auto url = u"Back/" + bS + u".img/" + u"ani" + u"/" + std::u16string{no_str.begin(), no_str.end()};
        AnimatedSprite *aspr = new AnimatedSprite(Wz::Map->get_root()->find_from_path(url), a);
        bspr.spr = aspr;
        break;
    }
    default:
        break;
    }
}