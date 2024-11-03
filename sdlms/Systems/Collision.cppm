module;

#include <SDL3/SDL.h>
#include "entt/entt.hpp"

export module systems:collision;

import components;
import core;
import commons;

export bool collision(SDL_FRect m_rect, Transform *m_tr, SDL_FRect n_rect, Transform *n_tr);

export bool collision(Mob *mob, Transform *tr);

export bool collision(Trap *trap, AnimatedSprite *aspr, Transform *tr);

export bool collision(AttackWarp *atkw, Mob *mob, Transform *m_tr);

export bool collision(AttackWarp *atkw, Npc *npc, Transform *n_tr);

export bool collision(AttackWarp *atkw, Character *cha, Transform *c_tr);

export bool collision(Mob *mob, Transform *m_tr, SDL_FRect n_rect, Transform *n_tr);

export bool collision(Npc *npc, Transform *m_tr, SDL_FRect n_rect, Transform *n_tr);

export bool collision(Character *cha, Transform *c_tr, SDL_FRect n_rect, Transform *n_tr);

export bool collision(Triangle t, Transform *t_tr, SDL_FRect n_rect, Transform *n_tr);

export bool collision(Mob *mob, Transform *m_tr, Triangle t, Transform *n_tr);

export bool collision(Character *cha, Transform *c_tr, Triangle t, Transform *n_tr);
