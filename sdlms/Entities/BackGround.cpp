#include "BackGround.h"
#include "Resource/Wz.h"
#include "Components/Sprite.h"
#include "Components/AnimatedSprite.h"
#include "Components/HVTile.h"

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

    switch (type)
    {
    case HTILED:
    case HMOVEA:
        auto hvtile = new HVTile(cx, -1);
        break;
    case VTILED:
    case VMOVEA:
        auto hvtile = new HVTile(-1, cy);
        break;
    case TILED:
    case HMOVEB:
    case VMOVEB:
        auto hvtile = new HVTile(cx, cy);
        break;
    }
    // switch (_type)
    // {
    // case HMOVEA:
    // case HMOVEB:
    //     hspeed = 1;
    //     break;
    // case VMOVEA:
    // case VMOVEB:
    //     vspeed = 1;
    //     break;
    // }

    switch (ani)
    {
    case 0:
    {
        auto url = u"Back/" + bS + u".img/" + u"back" + u"/" + std::u16string{no_str.begin(), no_str.end()};
        Sprite *spr = Sprite::load_sprite(world->get_resource<Wz>().Map->get_root()->find_from_path(url));
        break;
    }
    case 1:
    {
        auto url = u"Back/" + bS + u".img/" + u"ani" + u"/" + std::u16string{no_str.begin(), no_str.end()};
        AnimatedSprite *aspr = AnimatedSprite::load_animated_sprite(world->get_resource<Wz>().Map->get_root()->find_from_path(url));
        break;
    }
    default:
        break;
    }
}