#include "Obj.h"
#include "wz/Property.hpp"
#include "entt/entt.hpp"
#include "Core/Core.h"
#include "Components/Components.h"
#include "Commons/Commons.h"
#include "Resources/Wz.h"

void load_obj(wz::Node *node, int id, int layer)
{
    auto ent = World::registry->create();

    auto oS = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"oS"))->get();
    auto l0 = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"l0"))->get();
    auto l1 = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"l1"))->get();
    auto l2 = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"l2"))->get();
    auto url = u"Obj/" + oS + u".img/" + l0 + u"/" + l1 + u"/" + l2;

    auto x = dynamic_cast<wz::Property<int> *>(node->get_child(u"x"))->get();
    auto y = dynamic_cast<wz::Property<int> *>(node->get_child(u"y"))->get();
    auto z = dynamic_cast<wz::Property<int> *>(node->get_child(u"z"))->get();

    auto flip = dynamic_cast<wz::Property<int> *>(node->get_child(u"f"))->get();

    if (auto d = Wz::Map->get_root()->find_from_path(url)->get_child(u"damage"))
    {
        World::registry->emplace<Trap>(ent, dynamic_cast<wz::Property<int> *>(d)->get());
    }

    auto asprw = AnimatedSprite::Wrap::load(Wz::Map->get_root()->find_from_path(url));
    World::registry->emplace<AnimatedSprite>(ent, asprw);
    World::registry->emplace<Transform>(ent, (float)x, (float)y, layer * LAYER_Z + z * 100 + id + OBJ_Z, flip);
}