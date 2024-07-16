module;

#include "wz/Property.hpp"
#include <any>

module entities;

import components;
import resources;
import core;

void load_tile(wz::Node *node, const std::u16string &ts, int layer, int id)
{
    const int z_index = 10000;

    auto ent = World::registry.create();

    auto u = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"u"))->get();
    auto no = dynamic_cast<wz::Property<int> *>(node->get_child(u"no"))->get();
    auto x = dynamic_cast<wz::Property<int> *>(node->get_child(u"x"))->get();
    auto y = dynamic_cast<wz::Property<int> *>(node->get_child(u"y"))->get();

    auto no_str = std::to_string(no);

    auto url = u"Tile/" + ts + u".img/" + u + u"/" + std::u16string{no_str.begin(), no_str.end()};

    auto spr = World::registry.emplace<Sprite>(ent, Wz::Map->get_root()->find_from_path(url));
    World::registry.emplace<Transform>(ent, x, y, layer * 3000000 + std::any_cast<int>(spr.z) * 1000 + id + z_index);
}