module;

#include "entt/entt.hpp"

export module systems:ball;

import components;

export void ball_run();

bool ball_fall(entt::entity ent, Ball *ball, float delta_time);

bool ball_target(entt::entity src, Ball *ball, float delta_time);

void ball_hit(entt::entity src, entt::entity target);
