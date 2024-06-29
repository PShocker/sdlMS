#include "Character.h"
#include "NameTag.h"
#include "String.h"
#include "ChatBalloon.h"

#include "Components/Avatar.h"
#include "Components/Transform.h"
#include "Components/LimitTransform.h"
#include "Components/RelativeTransform.h"
#include "Resource/Wz.h"

#include <optional>

Character::Character(World *world, SDL_FPoint p)
{
    auto ava = Avatar::load();
    ava->add_head(u"00012006");
    ava->add_body(u"00002006");
    ava->add_coat(u"01040036");
    ava->add_pants(u"01060026");
    ava->add_face(u"00020000");
    ava->add_hairs(u"00030000");
    ava->add_cap(u"01000016");
    ava->add_shoes(u"01070002");
    ava->add_weapon(u"01302009");
    ava->add_shield(u"01092030");

    add_component(ava);
    world->add_component(ava);

    Transform *tr = new Transform(p);
    add_component(tr);
    world->add_component(tr, 3000000);

    // 限制人物移动范围
    LimitTransform *ltr = new LimitTransform(tr, std::nullopt, std::nullopt);
    add_component(ltr);
    world->add_component(ltr);

    auto str = new String(u"逆向萌新");
    add_entity(str);
    auto spr = str->get_component<Sprite>();

    auto nam = new NameTag(spr->width, spr->height, u"13");
    add_entity(nam);
    auto rtr = new RelativeTransform(tr, SDL_FPoint{(float)(-nam->get_width() / 2), (float)6});
    nam->add_component(rtr);
    nam->add_component(new Transform());
    world->add_component(rtr, 0);

    rtr = new RelativeTransform(tr, SDL_FPoint{(float)(-spr->get_width() / 2), (float)6 + (nam->get_height() - spr->height) / 2});
    str->add_component(rtr);
    str->add_component(new Transform());
    world->add_component(rtr, 0);

}

void Character::switch_act(uint8_t action)
{
    auto ava = get_component<Avatar>();
    if (action != ava->act)
    {
        ava->act_index = 0;
        ava->act_time = 0;
        ava->act = action;
    }
}

void Character::stop_animate()
{
    get_component<Avatar>()->animate = false;
}

void Character::start_animate()
{
    get_component<Avatar>()->animate = true;
}

Character::~Character()
{
    auto world = World::get_world();
    auto ava = get_component<Avatar>();
    world->destroy_component(ava, false);
    delete ava;
}