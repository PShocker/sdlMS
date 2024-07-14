#include "Obj.h"

#include "../Core/World.h"
#include "../Components/Sprite.h"
#include "../Components/AnimatedSprite.h"
#include "../Components/Transform.h"
#include "../Resource/Wz.h"

void Obj::load(wz::Node *node, int id, int layer)
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

    auto aspr = World::registry.emplace<AnimatedSprite>(ent, Wz::Map->get_root()->find_from_path(url));
    World::registry.emplace<Transform>(ent, x, y, layer * 3000000 + z * 100 + id + z_index);
}