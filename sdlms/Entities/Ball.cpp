#include "Ball.h"

#include "entt/entt.hpp"

#include "Core/Core.h"
#include "Components/Components.h"
#include "Commons/Commons.h"
#include "Resources/Wz.h"

void load_ball(AnimatedSpriteWarp *asprw, Transform *tran, entt::entity owner)
{
    auto ent = World::registry->create();
    auto &tr = World::registry->emplace<Transform>(ent);

    tr.position.y = tran->position.y - 30;

    tr.flip = tran->flip;
    tr.z = tran->z - 1;

    auto &ball = World::registry->emplace<Ball>(ent);
    ball.owner = owner;

    World::registry->emplace<Animated>(ent);
    World::registry->emplace<AnimatedSprite>(ent, asprw);

    auto &mv = World::registry->emplace<Move>(ent);
    mv.vspeed = 0;

    auto flip = tran->flip;
    if (flip)
    {
        mv.hspeed = 550;
        tr.position.x = tran->position.x;
    }
    else
    {
        mv.hspeed = -550;
        tr.position.x = tran->position.x;
    }
    World::zindex = true;
}

void load_ball(unsigned char num)
{
    auto ent = Player::ent;

    auto tr = World::registry->try_get<Transform>(ent);
    auto weaponinfo = World::registry->try_get<WeaponInfo>(ent);

    std::u16string ball_path;
    if (weaponinfo->attack == WeaponInfo::Attack::BOW)
    {
        ball_path = u"Consume/0207.img/02060001/bullet";
        auto node = Wz::Item->get_root()->find_from_path(ball_path);
        auto asprw = AnimatedSpriteWarp::load(node);
        for (size_t i = 0; i < num; i++)
        {
            Transform t(tr->position, tr->z, tr->flip);
            t.position.y = tr->position.x + i * 30;
            load_ball(asprw, &t, ent);
        }
    }
    else if (weaponinfo->attack == WeaponInfo::Attack::CLAW)
    {
        ball_path = u"Consume/0207.img/02070006/bullet";
        auto node = Wz::Item->get_root()->find_from_path(ball_path);
        auto asprw = AnimatedSpriteWarp::load(node);
        for (size_t i = 0; i < num; i++)
        {
            Transform t(tr->position, tr->z, tr->flip);
            auto flip = tr->flip;
            if (flip)
            {
                t.position.x = tr->position.x + i * 80;
            }
            else
            {
                t.position.x = tr->position.x - i * 80;
            }
            load_ball(asprw, &t, ent);
        }
    }
}
