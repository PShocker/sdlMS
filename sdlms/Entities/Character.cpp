module;

#include "entt/entt.hpp"

module entities;

import components;
import core;
import resources;

void load_character(float x, float y, bool sp, entt::entity *ent)
{
    auto cha = &World::registry->emplace<Character>(*ent);

    cha->add_head(u"00012000");
    cha->add_body(u"00002000");
    cha->add_coat(u"01040002");
    cha->add_pants(u"01060002");
    cha->add_face(u"00020000");
    cha->add_hairs(u"00030000");
    cha->add_cap(u"01002152");
    cha->add_shoes(u"01070002");
    cha->add_cape(u"01102053");
    cha->add_weapon(u"01402037");
    World::registry->emplace<WeaponInfo>(*ent, u"01402037");

    // cha.add_shield(u"01092030");

    World::registry->emplace<Attack>(*ent);
    World::registry->emplace<Animated>(*ent);
    World::registry->emplace<Effect>(*ent);
    World::registry->emplace<Damage>(*ent);
    if (sp)
    {
        World::registry->emplace<Transform>(*ent, recent_portal(x, y), 99999999);
    }
    else
    {
        World::registry->emplace<Transform>(*ent, x, y, 99999999);
    }
    World::registry->emplace<Move>(*ent);
    World::zindex = true;
    return;
}