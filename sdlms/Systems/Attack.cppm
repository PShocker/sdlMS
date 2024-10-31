module;

#include "entt/entt.hpp"
#include <SDL3/SDL.h>

export module systems:attack;

import components;
import core;

export void attack_run();

void attacking(AttackWarp *atk);

export void hit_effect(AttackWarp *atkw, SDL_FPoint *head, entt::entity ent, char type, int damage, int count = 1);

export void hit_effect(AttackWarp *atkw, Mob *mob, entt::entity ent = Player::ent);

export void hit_effect(AttackWarp *atkw, Npc *npc, entt::entity ent = Player::ent);

export void hit_effect(AttackWarp *atkw, Character *cha, entt::entity ent = Player::ent);
