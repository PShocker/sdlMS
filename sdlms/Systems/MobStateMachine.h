#pragma once

#include "entt/entt.hpp"
#include "Components/Components.h"

void mob_statemachine_run();

void mob_statemachine(entt::entity ent, float delta_time);

void mob_flip(Move *mv, Transform *tr);

int mob_move(Mob *mob, Move *mv, Transform *tr, int state, float delta_time);
int mob_fly(Mob *mob, Move *mv, Transform *tr, int state, float delta_time);
void mob_action(Mob *mob, Move *mv, Transform *tr, int state, int new_state);
optional<int> mob_hit(Attack *atk, entt::entity ent, std::optional<SDL_FPoint> hit_point);
bool mob_hit_move(std::optional<SDL_FPoint> &point, entt::entity ent);
void mob_drop(Mob *mob, Transform *tr);
bool mob_fall(entt::entity ent, float delta_time);
int mob_active(Mob *mob, Move *mv, Transform *tr, int state, float delta_time);
bool mob_revive(entt::entity ent, float delta_time);
bool mob_attack(Mob *mob, Transform *tr);
bool mob_jump(Mob *mob, Move *mv);
bool mob_collision_attack(entt::entity ent);
void mob_set_hspeed(Mob *mob, Move *mv, Transform *tr);
void mob_set_vspeed(Mob *mob, Move *mv, Transform *tr);