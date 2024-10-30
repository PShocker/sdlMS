module;

#include "entt/entt.hpp"

export module systems:ball;

import components;

export void ball_run();

bool ball_fall(Ball *ball, entt::entity *ent, float delta_time);

bool ball_target_mob(entt::entity *ball, entt::entity *target, bool multiple);
