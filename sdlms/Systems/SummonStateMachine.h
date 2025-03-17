#pragma once
#include "Components/Components.h"
#include "entt/entt.hpp"

void summon_statemachine_run();

void summon_statemachine(entt::entity ent, float delta_time);

void summon_flip(Move *mv, Transform *tr);

entt::entity summon_attack(Transform *tr);
bool summon_attack_action(entt::entity ent, entt::entity target);

bool summon_move(entt::entity ent, int state, float delta_time);
bool summon_fall(entt::entity ent, float delta_time);
void summon_fly(entt::entity ent, int state, float delta_time);
int summon_follow(entt::entity ent);
int summon_action(Summon *sum, int state, int new_state);
bool summon_jump(Summon *sum, Transform *tr, Move *mv);

void summon_set_hspeed(entt::entity ent, entt::entity target, float distance, float speed = 1);
void summon_set_vspeed(entt::entity ent, entt::entity target, float distance, float speed = 1);
