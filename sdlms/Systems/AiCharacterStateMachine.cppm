module;

#include "entt/entt.hpp"

export module systems:aicharacterstatemachine;

import components;

export void aicharacter_statemachine_run();

void aicharacter_statemachine(entt::entity ent, float delta_time);

bool aicharacter_fall(Move *mv, Transform *tr, float delta_time);

void aicharacter_action(Character *cha, int state, int new_state, Move *mv, entt::entity ent);

bool aicharacter_hit(Hit *hit, entt::entity ent);

void aicharacter_cooldown(Character *cha, int delta_time);

void aicharacter_drop(Character *cha, Transform *tr);
