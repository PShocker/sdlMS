#include "Tile.h"
#include "Components/Sprite.h"
#include "Components/Transform.h"
#include "Resource/Wz.h"
#include <format>
#include <string>

Tile::Tile(wz::Node *node, const std::u16string &ts, int layer, int id, World *world)
{
    auto u = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"u"))->get();
    auto no = dynamic_cast<wz::Property<int> *>(node->get_child(u"no"))->get();
    auto x = dynamic_cast<wz::Property<int> *>(node->get_child(u"x"))->get();
    auto y = dynamic_cast<wz::Property<int> *>(node->get_child(u"y"))->get();

    auto no_str = std::to_string(no);

    auto url = u"Tile/" + ts + u".img/" + u + u"/" + std::u16string{no_str.begin(), no_str.end()};

    Sprite *spr = new Sprite(world->get_resource<Wz>().Map->get_root()->find_from_path(url));
    Transform *t = new Transform{(float)x, (float)y};
    add_component(t);
    add_component(spr);
    world->add_component(t, layer * 300000 + std::any_cast<int>(spr->z) * 1000 + id + 10000);
}

Tile::~Tile()
{
    auto world = World::get_world();

    auto spr = get_component<Sprite>();
    delete spr;

    auto t = get_component<Transform>();
    world->destroy_component(t, false);
    delete t;
}
