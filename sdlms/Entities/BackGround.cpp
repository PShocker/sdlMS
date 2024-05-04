#include "BackGround.h"
#include "Resource/Wz.h"
#include "Components/Sprite.h"
#include "Components/AnimatedSprite.h"
#include "Components/HVTile.h"
#include "Components/HVMove.h"
#include "Components/Transform.h"

BackGround::BackGround(wz::Node *node, int id, World *world)
{
    auto bS = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"bS"))->get();
    if (bS == u"")
    {
        return;
    }
    auto ani = dynamic_cast<wz::Property<int> *>(node->get_child(u"ani"))->get();

    auto x = dynamic_cast<wz::Property<int> *>(node->get_child(u"x"))->get();
    auto y = dynamic_cast<wz::Property<int> *>(node->get_child(u"y"))->get();

    auto cx = dynamic_cast<wz::Property<int> *>(node->get_child(u"cx"))->get();
    auto cy = dynamic_cast<wz::Property<int> *>(node->get_child(u"cy"))->get();

    auto rx = dynamic_cast<wz::Property<int> *>(node->get_child(u"rx"))->get();
    auto ry = dynamic_cast<wz::Property<int> *>(node->get_child(u"ry"))->get();

    auto type = dynamic_cast<wz::Property<int> *>(node->get_child(u"type"))->get();

    auto no = dynamic_cast<wz::Property<int> *>(node->get_child(u"no"))->get();

    auto front = dynamic_cast<wz::Property<int> *>(node->get_child(u"front"))->get();

    auto flip = dynamic_cast<wz::Property<int> *>(node->get_child(u"f"))->get();

    auto no_str = std::to_string(no);

    HVTile *hvt = nullptr;
    HVMove *hvm = nullptr;

    Transform *t = new Transform{(float)x, (float)y};
    add_component(t);

    switch (type)
    {
    case NORMAL:
        hvt = new HVTile(cx, cy, false, false);
        break;
    case HTILED:
    case HMOVEA:
        hvt = new HVTile(cx, cy, true, false);
        break;
    case VTILED:
    case VMOVEA:
        hvt = new HVTile(cx, cy, false, true);
        break;
    case TILED:
    case HMOVEB:
    case VMOVEB:
        hvt = new HVTile(cx, cy, true, true);
        break;
    }
    switch (type)
    {
    case HMOVEA:
    case HMOVEB:
        hvm = new HVMove(rx, ry, true, false);
        break;
    case VMOVEA:
    case VMOVEB:
        hvm = new HVMove(rx, ry, false, true);
        break;
    default:
        hvm = new HVMove(rx, ry, false, false);
        break;
    }
    add_component(hvt);
    add_component(hvm);

    switch (ani)
    {
    case 0:
    {
        auto url = u"Back/" + bS + u".img/" + u"back" + u"/" + std::u16string{no_str.begin(), no_str.end()};
        Sprite *spr = Sprite::load_sprite(world->get_resource<Wz>().Map->get_root()->find_from_path(url));
        add_component(spr);
        world->add_component(t, id - 1000);
        break;
    }
    case 1:
    {
        auto url = u"Back/" + bS + u".img/" + u"ani" + u"/" + std::u16string{no_str.begin(), no_str.end()};
        AnimatedSprite *aspr = AnimatedSprite::load_animated_sprite(world->get_resource<Wz>().Map->get_root()->find_from_path(url));
        add_component(aspr);
        world->add_component(t, id - 1000);
        world->add_unique_component(aspr);
        break;
    }
    default:
        break;
    }
}