#pragma once

#include "entt/entt.hpp"
#include "Components/Components.h"

void pet_statemachine_run();

void pet_statemachine(entt::entity ent, float delta_time);

void pet_flip(Move *mv, Transform *tr);

bool pet_move(entt::entity ent, int state, float delta_time);
bool pet_fall(entt::entity ent, float delta_time);
bool pet_jump(Pet *pet,Transform *tr, Move *mv);

void pet_pick(entt::entity ent);
int pet_follow(entt::entity ent);
bool pet_climb(entt::entity ent);

int pet_action(Pet *pet, int state, int new_state);
