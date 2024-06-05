#include "Character.h"
#include "Components/Avatar.h"
#include "Resource/Wz.h"

Character::Character(World *world)
{
    auto ava = Avatar::load();
    ava->add_head(u"00012006");
    ava->add_body(u"00002006");
    ava->add_arm(u"00002006");
    ava->add_coat(u"01040036");
    ava->add_coat_arm(u"01040036");
    ava->add_pants(u"01060026");
    ava->add_face(u"00020000");
    ava->add_hairs(u"00030000");
    // ava->add_shoes(u"01070002");
    // ava->add_weapon(u"01442028");

    ava->add_hand(u"00002006");
    ava->add_lHand(u"00002006");
    ava->add_rHand(u"00002006");

    add_component(ava);
    world->add_component(ava);
}

Character::~Character()
{
    auto world = World::get_world();
    auto ava = get_component<Avatar>();
    world->destroy_component(ava, false);
    delete ava;
}
