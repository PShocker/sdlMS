module;

#include <SDL3/SDL.h>
#include <optional>
#include "entt/entt.hpp"

export module systems:statemachine;

import components;

export void statemachine_run();
void player_statemachine(entt::entity *ent);
void player_flip(Transform *tr);
int player_walk(Move *mv, Transform *tr, FootHold *foo);
bool player_fall(Move *mv, Transform *tr, FootHold *foo, entt::entity *ent);
int player_jump(Move *mv, Transform *tr, int state);
int player_attack(Move *mv, Character *cha, Transform *tr, int state);
bool player_attacking(Move *mv, Character *cha, Transform *tr, FootHold *foo, entt::entity *ent);

void player_action(Character *cha, int action);

std::optional<SDL_FPoint> intersect(SDL_FPoint p1,
                                    SDL_FPoint p2,
                                    SDL_FPoint p3,
                                    SDL_FPoint p4);
