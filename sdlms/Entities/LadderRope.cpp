module;

#include "wz/Property.hpp"

module entities;

import components;
import resources;
import core;

void load_ladderRope(wz::Node *node, int id)
{
    auto ent = World::registry.create();

    auto &lr = World::registry.emplace<LadderRope>(ent);

    lr.l = dynamic_cast<wz::Property<int> *>(node->get_child(u"l"))->get();
    lr.uf = dynamic_cast<wz::Property<int> *>(node->get_child(u"uf"))->get();
    lr.page = dynamic_cast<wz::Property<int> *>(node->get_child(u"page"))->get();

    lr.x = dynamic_cast<wz::Property<int> *>(node->get_child(u"x"))->get();
    lr.y1 = dynamic_cast<wz::Property<int> *>(node->get_child(u"y1"))->get();
    lr.y2 = dynamic_cast<wz::Property<int> *>(node->get_child(u"y2"))->get();

    lr.t = std::min(lr.y1, lr.y2);
    lr.b = std::max(lr.y1, lr.y2);

    LadderRope::lrs[id] = &lr;
}