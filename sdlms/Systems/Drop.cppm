module;

export module systems:drop;

import components;

export void drop_run();

bool drop_fall(Move *mv, Transform *tr, float delta_time);

void drop_pick(Move *mv, Transform *tr, Drop *dro, float delta_time);