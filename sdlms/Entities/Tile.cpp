#include "Tile.h"
#include "Components/Sprite.h"
#include "Components/Transform.h"
#include <format>

Tile::Tile(wz::Node *node, std::u16string ts)
{
    auto u = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"u"))->get();
    auto no = dynamic_cast<wz::Property<int> *>(node->get_child(u"no"))->get();
    auto x = dynamic_cast<wz::Property<int> *>(node->get_child(u"x"))->get();
    auto y = dynamic_cast<wz::Property<int> *>(node->get_child(u"y"))->get();

    auto no_str = std::to_string(no);
    auto _no = std::u16string{no_str.begin(), no_str.end()};

    auto url = u"Tile/" + ts + u".img/" + u + u"/" + _no;

    Sprite *spr = Sprite::load_sprite(node);
    Transform *t = new Transform{(float)x, (float)y};

    add_component(t);
    add_component(spr);
}