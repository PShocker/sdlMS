module;

#include "entt/entt.hpp"
#include <SDL3/SDL.h>

export module systems:attack;

import components;

export void attack_run();

void player_attack(Attack *atk, Transform *tr);

void mob_collision(Mob *mob, Transform *tr);

void attack_mob(Attack *atk, Transform *tr, Mob *mob, SDL_FRect *atk_rect, SDL_FPoint *pos, entt::entity *ent);

void attack_npc(Attack *atk, Transform *tr, Npc *npc, SDL_FRect *atk_rect, SDL_FPoint *pos, entt::entity *ent);
