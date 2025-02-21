#pragma once

#include "entt/entt.hpp"
#include "Components/Components.h"

void ball_run();

entt::entity ball_fall(entt::entity ent, Ball *ball);

bool ball_track(entt::entity src, Ball *ball, float delta_time);

bool ball_no_track(entt::entity src, Ball *ball);

bool ball_move(entt::entity src, Ball *ball, float delta_time);

void ball_hit(entt::entity src, Ball *ball, entt::entity target);

bool ball_distance(entt::entity src, entt::entity target, float &dx, float &dy);

float ball_rotation(Ball *ball, Transform *tr);
