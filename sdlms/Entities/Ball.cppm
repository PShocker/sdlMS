module;

#include "entt/entt.hpp"

export module entities:ball;

import components;

export
{
    void load_ball(Skill * ski, Transform * tran, entt::entity * owner);
};
