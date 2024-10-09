module;

export module systems:drop;

import components;

export void drop_run();

bool drop_fall(Move *mv, Transform *tr, float delta_time);

bool drop_pick(Move *mv, Transform *tr, AnimatedSprite *aspr, float delta_time);
