#include "Character.h"
#include "Components/Avatar.h"
#include "Resource/Wz.h"

Character::Character(World *world)
{
    auto ava = Avatar::load_avatar(world->get_resource<Wz>().Character->get_root());
    ava->add_head();
    ava->add_body();
    ava->add_arm();
    ava->add_coat(u"01040002");
    ava->add_pants(u"01060002");
    ava->add_face(u"00020000");
    ava->add_hairs(u"00030000");
    ava->add_lHand();
    ava->add_rHand();

    add_component(ava);
    world->add_component(ava);
}