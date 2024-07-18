module;

export module systems:move;

import components;

export void move_run();
void move_player(Move *mv, Transform *tr, Character *cha);
void move_play_flip(Transform *tr);
void move_play_walk(Move *mv,Transform *tr);
void move_play_fall(Move *mv,Transform *tr);
