module;

#include <SDL3/SDL.h>
#include <optional>
#include "entt/entt.hpp"

export module systems:statemachine;

import components;

export void statemachine_run();
void player_statemachine(entt::entity *ent, float delta_time);

void player_flip(Transform *tr);

int player_walk(Move *mv, Transform *tr, float delta_time);

bool player_fall(Move *mv, Transform *tr, entt::entity *ent, float delta_time);
bool player_jump(Move *mv, Transform *tr);

int player_attack(Move *mv, Character *cha, Transform *tr, int state, entt::entity *ent);
bool player_attacking(Move *mv, Character *cha, Transform *tr, entt::entity *ent, float delta_time);
bool player_attack_afterimage(entt::entity *ent);

bool player_climb(Move *mv, Transform *tr, int state);
int player_climbing(Character *cha, Move *mv, Transform *tr, entt::entity *ent, float delta_time);

bool player_prone(Move *mv, Transform *tr, int state);
bool player_proning();

void player_border_limit(Move *mv, Transform *tr);
void player_action(Character *cha, int state, int new_state, Move *mv);

std::optional<SDL_FPoint> intersect(SDL_FPoint p1,
                                    SDL_FPoint p2,
                                    SDL_FPoint p3,
                                    SDL_FPoint p4);
