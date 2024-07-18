module;

#include <SDL3/SDL.h>
#include <optional>

export module systems:move;

import components;

export void move_run();
void move_player(Move *mv, Transform *tr, Character *cha, FootHold *foo);
void move_play_flip(Transform *tr);
bool move_play_walk(Move *mv, Transform *tr, FootHold *foo);
bool move_play_fall(Move *mv, Transform *tr, FootHold *foo);

std::optional<SDL_FPoint> intersect(SDL_FPoint p1,
                                    SDL_FPoint p2,
                                    SDL_FPoint p3,
                                    SDL_FPoint p4);
