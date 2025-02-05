#include "Ball.h"

#include "entt/entt.hpp"

#include "Core/Core.h"
#include "Components/Components.h"
#include "Commons/Commons.h"
#include "Resources/Wz.h"

void load_ball(Skill *ski, Transform *tran, entt::entity owner)
{
    auto ent = World::registry->create();
    auto &tr = World::registry->emplace<Transform>(ent);

    tr.position.y = tran->position.y - 30;

    tr.flip = tran->flip;
    tr.z = tran->z - 1;

    auto &s = World::registry->emplace<Skill>(ent);
    s.ski = ski->ski;

    auto &ball = World::registry->emplace<Ball>(ent);
    ball.owner = owner;

    World::registry->emplace<Animated>(ent);
    World::registry->emplace<AnimatedSprite>(ent, ski->ski->ball);

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