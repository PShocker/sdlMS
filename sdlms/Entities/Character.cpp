#include "Character.h"
#include "NameTag.h"
#include "String.h"

#include "Components/Avatar.h"
#include "Components/Transform.h"
#include "Components/RelativeTransform.h"
#include "Resource/Wz.h"

Character::Character(World *world, SDL_FPoint p)
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
    ava->add_shoes(u"01070002");
    ava->add_weapon(u"01442028");

    ava->add_hand(u"00002006");
    ava->add_lHand(u"00002006");
    ava->add_rHand(u"00002006");

    add_component(ava);
    world->add_component(ava);

    Transform *tr = new Transform(p);
    add_component(tr);
    world->add_component(tr, 3000000);

    auto str = new String(u"逆向萌新");
    add_entity(str);
    auto spr = str->get_component<Sprite>();
    {
        auto rtr = new RelativeTransform(tr, SDL_FPoint{(float)(-spr->get_width() / 2 + 2), (float)9});
        str->add_component(rtr);
        str->add_component(new Transform());
        world->add_component(rtr, 1);
    }
    {
        auto nam = new NameTag(spr->width + 4, spr->height + 6);
        add_entity(nam);
        auto rtr = new RelativeTransform(tr, SDL_FPoint{(float)(-spr->get_width() / 2), (float)6});
        nam->add_component(rtr);
        nam->add_component(new Transform());
        world->add_component(rtr, 0);
    }
}

Character::~Character()
{
    auto world = World::get_world();
    auto ava = get_component<Avatar>();
    world->destroy_component(ava, false);
    delete ava;
}
