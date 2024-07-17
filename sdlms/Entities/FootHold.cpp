module;

#include "wz/Property.hpp"

module entities;

import components;
import resources;
import core;

void load_foothold(wz::Node *node, int page, int zmass, int id)
{
    auto ent = World::registry.create();

    auto &fh = World::registry.emplace<FootHold>(ent);
    fh.next = dynamic_cast<wz::Property<int> *>(node->get_child(u"next"))->get();
    fh.prev = dynamic_cast<wz::Property<int> *>(node->get_child(u"prev"))->get();
    fh.x1 = dynamic_cast<wz::Property<int> *>(node->get_child(u"x1"))->get();
    fh.x2 = dynamic_cast<wz::Property<int> *>(node->get_child(u"x2"))->get();
    fh.y1 = dynamic_cast<wz::Property<int> *>(node->get_child(u"y1"))->get();
    fh.y2 = dynamic_cast<wz::Property<int> *>(node->get_child(u"y2"))->get();

    FootHold::fhs[id] = &fh;
}