#include "Character.h"
#include "Portal.h"
#include "Pet.h"

#include "entt/entt.hpp"

#include "Core/Core.h"
#include "Components/Components.h"
#include "Commons/Commons.h"
#include "Resources/Wz.h"

void load_character(float x, float y, bool sp, const std::u16string &name, entt::entity ent)
{
    auto cha = &World::registry->emplace<Character>(ent);

    cha->add_head(u"00012000");
    cha->add_body(u"00002000");
    cha->add_coat(u"01040002");
    cha->add_pants(u"01060002");
    cha->add_face(u"00020000");
    cha->add_hairs(u"00030000");
    // cha->add_cap(u"01002068");
    // cha->add_shoes(u"01070002");
    // cha->add_cape(u"01102053");
    // cha->add_shield(u"01092049");
    std::u16string weapon_id = u"01452000";
    cha->add_weapon(weapon_id);
    World::registry->emplace<WeaponWrap>(ent, weapon_id);
    World::registry->emplace<Effect>(ent);
    World::registry->emplace<Damage>(ent);
    auto count = World::registry->view<Character>().size();

    if (sp)
    {
        World::registry->emplace<Transform>(ent, recent_portal(x, y), LAYER_Z * 8 + CHARACTER_Z - count);
    }
    else
    {
        World::registry->emplace<Transform>(ent, x, y, LAYER_Z * 8 + CHARACTER_Z - count);
    }
    World::registry->emplace<Move>(ent);
    auto buff = &World::registry->emplace<Buff>(ent);
    buff->owner = ent;

    if (name.size() > 0)
    {
        auto nametag = &World::registry->emplace<NameTag>(ent);
        NameTag::push(nametag, Player::name, SDL_Color{90, 148, 247, 255});
    }

    World::zindex = true;
    return;
}