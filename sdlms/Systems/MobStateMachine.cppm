module;

#include "entt/entt.hpp"

export module systems:mobstatemachine;

import components;

export void mob_statemachine_run();

void mob_statemachine(entt::entity *ent, float delta_time);

void mob_flip(Move *mv, Transform *tr);

int mob_move(Mob *mob, Move *mv, Transform *tr, int state, float delta_time);
void mob_action(Mob *mob, Move *mv, int state, int new_state);
bool mob_hit(Hit *hit, entt::entity *ent);
void mob_drop(Mob *mob, Transform *tr);
bool mob_fall(Move *mv, Transform *tr, float delta_time);
int mob_active(Mob *mob, Move *mv, int state,float delta_time);