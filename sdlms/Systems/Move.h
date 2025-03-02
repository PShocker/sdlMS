#pragma once

#include "Components/Components.h"

float move_border_limit_x(Move *mv, float x);

float move_border_limit_y(Move *mv, float x);

bool move_fall(Move *mv, Transform *tr, float delta_time, int z_index, bool fall_collide = true, bool wall_collide = true);

bool move_move(Move *mv, Transform *tr, float friction, float delta_time, bool fall = true);