module;

#include "wz/Property.hpp"

module entities;

import components;
import resources;
import core;
import commons;

void load_obj(wz::Node *node, int id, int layer)
{
    auto ent = World::registry.create();

    auto oS = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"oS"))->get();
    auto l0 = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"l0"))->get();
    auto l1 = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"l1"))->get();
    auto l2 = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"l2"))->get();
    auto url = u"Obj/" + oS + u".img/" + l0 + u"/" + l1 + u"/" + l2;

    auto x = dynamic_cast<wz::Property<int> *>(node->get_child(u"x"))->get();
    auto y = dynamic_cast<wz::Property<int> *>(node->get_child(u"y"))->get();
    auto z = dynamic_cast<wz::Property<int> *>(node->get_child(u"z"))->get();

    auto flip = dynamic_cast<wz::Property<int> *>(node->get_child(u"f"))->get();

    World::registry.emplace<AnimatedSprite>(ent, Wz::Map->get_root()->find_from_path(url));
    World::registry.emplace<Transform>(ent, x, y, layer * LAYER_Z + z * 100 + id + OBJ_Z, flip);
}