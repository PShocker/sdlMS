#pragma once

#include "entt/entt.hpp"
#include "Components/Components.h"

void ball_run();

entt::entity ball_fall(entt::entity ent);

bool ball_track(entt::entity src, float delta_time);

bool ball_no_track(entt::entity src, float delta_time);

void ball_target_point(entt::entity src, entt::entity target);

void ball_hit(entt::entity src, entt::entity target);

bool ball_collision(entt::entity src, entt::entity target);
