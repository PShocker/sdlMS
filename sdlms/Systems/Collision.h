#pragma once

#include <SDL3/SDL.h>
#include "entt/entt.hpp"
#include "Components/Components.h"
#include "Commons/Commons.h"

bool collision(SDL_FRect m_rect, Transform *m_tr, SDL_FRect n_rect, Transform *n_tr);

bool collision(Mob *mob, Transform *tr);

bool collision(Trap *trap, AnimatedSprite *aspr, Transform *tr);

bool collision(AttackWarp *atkw, Mob *mob, Transform *m_tr);

bool collision(AttackWarp *atkw, Npc *npc, Transform *n_tr);

bool collision(AttackWarp *atkw, Character *cha, Transform *c_tr);

bool collision(Mob *mob, Transform *m_tr, SDL_FRect n_rect, Transform *n_tr);

bool collision(Npc *npc, Transform *m_tr, SDL_FRect n_rect, Transform *n_tr);

bool collision(Character *cha, Transform *c_tr, SDL_FRect n_rect, Transform *n_tr);

bool collision(Triangle t, Transform *t_tr, SDL_FRect n_rect, Transform *n_tr);

bool collision(Mob *mob, Transform *m_tr, Triangle t, Transform *n_tr);

bool collision(Character *cha, Transform *c_tr, Triangle t, Transform *n_tr);
