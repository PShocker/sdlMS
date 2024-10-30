module;

#include "entt/entt.hpp"

export module systems:ball;

import components;

export void ball_run();

bool ball_fall(entt::entity *ent, Ball *ball, float delta_time);

bool ball_target_mob(entt::entity *src, Ball *ball, entt::entity *target, bool multiple);

bool ball_target(entt::entity *src, Ball *ball, float delta_time);