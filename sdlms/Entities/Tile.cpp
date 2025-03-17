#include "Tile.h"
#include "Core/Core.h"
#include "Components/Components.h"
#include "Commons/Commons.h"
#include "Resources/Wz.h"
#include "entt/entt.hpp"

void load_tile(wz::Node *node, const std::u16string &ts, int layer, int id)
{
    auto ent = World::registry->create();

    auto u = dynamic_cast<wz::Property<wz::wzstring> *>(node->get_child(u"u"))->get();
    auto no = dynamic_cast<wz::Property<int> *>(node->get_child(u"no"))->get();
    auto x = dynamic_cast<wz::Property<int> *>(node->get_child(u"x"))->get();
    auto y = dynamic_cast<wz::Property<int> *>(node->get_child(u"y"))->get();

    auto no_str = std::to_string(no);

    auto url = u"Tile/" + ts + u".img/" + u + u"/" + std::u16string{no_str.begin(), no_str.end()};
    auto sprw = SpriteWarp::load(Wz::Map->get_root()->find_from_path(url));

    auto &spr = World::registry->emplace<Sprite>(ent, sprw);
    World::registry->emplace<Transform>(ent, (float)x, (float)y,
                                        layer * LAYER_Z + dynamic_cast<wz::Property<int> *>(sprw->n->get_child(u"z"))->get() * 1000 + id + TILE_Z);
}