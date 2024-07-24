module;

#include "entt/entt.hpp"

module entities;

import components;
import core;
import resources;

void load_character(float x, float y)
{
    Player::ent = World::registry->create();

    World::registry->emplace<Transform>(Player::ent, x, y, 99999999);

    World::registry->emplace<Move>(Player::ent);

    auto cha = &World::registry->emplace<Character>(Player::ent);

    cha->add_head(u"00012000");
    cha->add_body(u"00002000");
    cha->add_coat(u"01040002");
    cha->add_pants(u"01060026");
    cha->add_face(u"00020000");
    cha->add_hairs(u"00030000");
    // cha.add_cap(u"01000016");
    cha->add_shoes(u"01070002");
    cha->add_weapon(u"01302060");
    // cha.add_shield(u"01092030");

    World::zindex = true;
}