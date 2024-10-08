module;

export module systems:move;

import components;

float move_border_limit(Move *mv, float x);

export bool move_fall(Move *mv, Transform *tr, float delta_time, bool fall_collide = true);

export bool move_move(Move *mv, Transform *tr, int friction, float delta_time);
