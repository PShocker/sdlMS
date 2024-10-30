module;

#include "entt/entt.hpp"

module entities;

import components;
import resources;
import core;

void load_ball(Skill *ski, Transform *tran, entt::entity *owner)
{
    auto ent = World::registry->create();
    auto &tr = World::registry->emplace<Transform>(ent);

    tr.position.x = tran->position.x;
    tr.position.y = tran->position.y;
    tr.flip = tran->flip;

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
        mv.hspeed = 100;
    }
    else
    {
        mv.hspeed = -100;
    }
}