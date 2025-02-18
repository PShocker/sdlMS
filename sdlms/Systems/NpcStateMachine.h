#pragma once

#include "entt/entt.hpp"
#include "Components/Components.h"

void npc_statemachine_run();

void npc_statemachine(entt::entity ent, float delta_time);

void npc_move(Move *mv, Transform *tr, float delta_time);
