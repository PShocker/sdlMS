module;

#include "entt/entt.hpp"

module entities;

import components;
import core;
import resources;

entt::entity *load_character()
{
    Player::ent = World::registry->create();

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

    World::registry->emplace<Animated>(Player::ent);
    World::registry->emplace<Effect>(Player::ent);

    World::zindex = true;
    return &Player::ent;
}