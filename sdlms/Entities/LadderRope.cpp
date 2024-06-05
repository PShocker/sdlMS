#include "LadderRope.h"
#include "Components/Line.h"
#include "Components/Physic/Normal.h"

LadderRope::LadderRope(wz::Node *node, World *world)
{
    l = dynamic_cast<wz::Property<int> *>(node->get_child(u"l"))->get();
    uf = dynamic_cast<wz::Property<int> *>(node->get_child(u"uf"))->get();
    page = dynamic_cast<wz::Property<int> *>(node->get_child(u"page"))->get();

    auto x = dynamic_cast<wz::Property<int> *>(node->get_child(u"x"))->get();
    auto y1 = dynamic_cast<wz::Property<int> *>(node->get_child(u"y1"))->get();
    auto y2 = dynamic_cast<wz::Property<int> *>(node->get_child(u"y2"))->get();

    auto *c = new Line{{(float)x, (float)y1}, {(float)x, (float)y2}};
    add_component(c);
}

LadderRope::~LadderRope()
{
    auto c = get_component<Line>();
    delete c;
}
