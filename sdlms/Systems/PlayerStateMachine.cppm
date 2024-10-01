module;

#include <SDL3/SDL.h>
#include <optional>
#include "entt/entt.hpp"

export module systems:playerstatemachine;

import components;

export void player_statemachine_run();
void player_statemachine(entt::entity *ent, float delta_time);

void player_flip(Transform *tr);

int player_walk(Move *mv, Transform *tr, float delta_time);

bool player_fall(Move *mv, Transform *tr, entt::entity *ent, float delta_time);
bool player_jump(Move *mv, Character *cha, Transform *tr, int state);
bool player_down_jump(Move *mv, Transform *tr);

int player_attack(Move *mv, Character *cha, Transform *tr, int state, entt::entity *ent);
bool player_attacking(Move *mv, Character *cha, Transform *tr, entt::entity *ent, float delta_time);

bool player_climb(Move *mv, Transform *tr, int state);
int player_climbing(Character *cha, Move *mv, Transform *tr, entt::entity *ent, float delta_time);

bool player_prone(Move *mv, Transform *tr, int state);
bool player_proning();

float player_border_limit(Move *mv, float x);
void player_action(Character *cha, int state, int new_state, Move *mv);

void player_portal(Move *mv, entt::entity *ent);

export int player_foothold_cooldown = 0;
export int player_portal_cooldown = 0;
export int player_alert_cooldown = 0;
export int player_ladderrope_cooldown = 0;
export int player_invincible_cooldown = 0;

void player_cooldown(int delta_time);

bool player_alert();

bool player_hit(Hit *hit, entt::entity *ent);

bool player_skill(Move *mv, Character *cha, Transform *tr, int state, entt::entity *ent);
bool player_skilling(Move *mv, Character *cha, Transform *tr, entt::entity *ent, float delta_time);

bool player_double_jump(Move *mv, Transform *tr, entt::entity *ent);