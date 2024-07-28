module;

#include "entt/entt.hpp"

export module systems:mobstatemachine;

import components;

export void mob_statemachine_run();

void mob_statemachine(entt::entity *ent, float delta_time);

void mob_flip(Move *mv, Transform *tr);

int mob_stand(Mob *mob, Move *mv, Transform *tr, int state, float delta_time);
int mob_move(Mob *mob, Move *mv, Transform *tr, int state, float delta_time);
void mob_action(Mob *mob, Move *mv, int state, int new_state);
export void mob_hit(Mob *mob, Transform *mob_tr, Transform *player_tr, Move *mv);
