#include "FootHold.h"
#include "Components/RigidLine.h"

FootHold::FootHold(wz::Node *node, int id, int page, int zmass, World *world) : id(id), page(page), zmass(zmass)
{
    auto foothold = node;

    next = dynamic_cast<wz::Property<int> *>(foothold->get_child(u"next"))->get();
    prev = dynamic_cast<wz::Property<int> *>(foothold->get_child(u"prev"))->get();
    auto x1 = dynamic_cast<wz::Property<int> *>(foothold->get_child(u"x1"))->get();
    auto x2 = dynamic_cast<wz::Property<int> *>(foothold->get_child(u"x2"))->get();
    auto y1 = dynamic_cast<wz::Property<int> *>(foothold->get_child(u"y1"))->get();
    auto y2 = dynamic_cast<wz::Property<int> *>(foothold->get_child(u"y2"))->get();

    RigidLine *r = new RigidLine{{(float)x1, (float)y1}, {(float)x2, (float)y2}};
    add_component(r);
}

FootHold::~FootHold()
{
    auto r = get_component<RigidLine>();
    delete r;
}
