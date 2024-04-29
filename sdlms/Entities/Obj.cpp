#include "Obj.h"
#include "Components/AnimatedSprite.h"
#include "Components/Transform.h"
#include "Resource/Wz.h"

Obj::Obj(wz::Node *node, int id, int layer, World *world)
{
    auto oS = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"oS"))->get();
    auto l0 = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"l0"))->get();
    auto l1 = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"l1"))->get();
    auto l2 = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"l2"))->get();
    auto url = u"Obj/" + oS + u".img/" + l0 + u"/" + l1 + u"/" + l2;

    auto x = dynamic_cast<wz::Property<int> *>(node->get_child(u"x"))->get();
    auto y = dynamic_cast<wz::Property<int> *>(node->get_child(u"y"))->get();
    auto z = dynamic_cast<wz::Property<int> *>(node->get_child(u"z"))->get();

    auto flip = dynamic_cast<wz::Property<int> *>(node->get_child(u"f"))->get();

    auto aspr = AnimatedSprite::load_animated_sprite(world->get_resource<Wz>().Map->get_root()->find_from_path(url));

    Transform *t = new Transform{(float)x, (float)y};

    add_component(t);
    add_component(aspr);
    world->add_component(t, 30000 * layer + z * 10 + id);
    world->add_unique_component(aspr);
}