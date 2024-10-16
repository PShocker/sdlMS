module;

#include "entt/entt.hpp"
#include <SDL3/SDL.h>

export module systems:attack;

import components;

export void attack_run();

void player_attack(AttackWarp *atk);

bool mob_collision(Mob *mob, Transform *tr);

bool trap_collision(Trap *trap, AnimatedSprite *aspr, Transform *tr);

bool attack_mob(AttackWarp *atk, Mob *mob, entt::entity *ent);

bool attack_npc(AttackWarp *atk, Npc *npc, entt::entity *ent);

bool attack_cha(AttackWarp *atk, Character *cha, entt::entity *ent);

bool collision(SpriteWarp *m_spr, Transform *m_tr, AttackWarp *n_atkw, Transform *n_tr);

// 怪物与人物碰撞
bool collision(SpriteWarp *m_spr, Transform *m_tr);

// 人物与人物攻击判断
bool collision(Character *m_cha, Transform *m_tr, AttackWarp *n_atk, Transform *n_tr);

void hit_effect(AttackWarp *atk, SDL_FPoint *head, entt::entity *ent, char type, int damage, int count = 1);