#pragma once
#include "Components/Components.h"
#include "entt/entt.hpp"

void summon_statemachine_run();

void summon_statemachine(entt::entity ent, float delta_time);

void summon_flip(Move *mv, Transform *tr);
entt::entity summon_attack(Transform *tr);
int summon_move(Summon *sum, Move *mv, Transform *tr, int state, float delta_time);
int summon_follow(Summon *sum, Move *mv, Transform *tr);
int summon_action(Summon *sum, int state, int new_state);
