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

    fh.page = page;
    fh.zmass = zmass;

    fh.next = dynamic_cast<wz::Property<int> *>(node->get_child(u"next"))->get();
    fh.prev = dynamic_cast<wz::Property<int> *>(node->get_child(u"prev"))->get();
    fh.x1 = dynamic_cast<wz::Property<int> *>(node->get_child(u"x1"))->get();
    fh.x2 = dynamic_cast<wz::Property<int> *>(node->get_child(u"x2"))->get();
    fh.y1 = dynamic_cast<wz::Property<int> *>(node->get_child(u"y1"))->get();
    fh.y2 = dynamic_cast<wz::Property<int> *>(node->get_child(u"y2"))->get();

    fh.l = std::min(fh.x1, fh.x2);
    fh.r = std::max(fh.x1, fh.x2);
    fh.t = std::min(fh.y1, fh.y2);
    fh.b = std::max(fh.y1, fh.y2);

    if (fh.x1 != fh.x2)
    {
        // 斜线
        fh.k = ((float)fh.y2 - (float)fh.y1) / ((float)fh.x2 - (float)fh.x1);
        fh.intercept = fh.y1 - fh.k.value() * fh.x1;
    }

    FootHold::fhs[id] = &fh;
}