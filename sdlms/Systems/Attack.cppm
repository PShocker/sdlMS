module;

#include "entt/entt.hpp"
#include <SDL3/SDL.h>

export module systems:attack;

import components;

export void attack_run();

void player_attack(Attack *atk);

bool mob_collision(Mob *mob, Transform *tr);

void attack_mob(Attack *atk, Mob *mob, entt::entity *ent);

void attack_npc(Attack *atk, Npc *npc, entt::entity *ent);

void attack_cha(Attack *atk, Character *cha, entt::entity *ent);

bool collision(SpriteWarp *m_spr, Transform *m_tr, Attack *n_atk, Transform *n_tr);

// 怪物与人物碰撞
bool collision(SpriteWarp *m_spr, Transform *m_tr);

// 人物与人物攻击判断
bool collision(Character *m_cha, Transform *m_tr, Attack *n_atk, Transform *n_tr);

void hit_effect(Attack *atk, SDL_FPoint *head, entt::entity *ent, char type, int damage);