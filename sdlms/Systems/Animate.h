#pragma once

#include "entt/entt.hpp"
#include "Components/Components.h"

void animate_run();

bool animate_sprite(AnimatedSprite *a);
void animate_character(Character *cha, entt::entity ent);
void animate_afterimage(AfterImage *aft, Character *cha, entt::entity ent);
void animate_effect(Effect *eff);
void animate_portal(Portal *por, entt::entity ent);
void animate_npc(Npc *npc, entt::entity ent);
void animate_mob(Mob *mob, entt::entity ent);
void animate_damage(Damage *dam);
void animate_tomb(Tomb *tomb, Transform *tr);
void animate_drop(Drop *dro, Transform *tr);
void animate_face(Character *cha);
void animate_reactor(Reactor *r);
void animate_install(Install *i);
void animate_ball(Ball *ball, entt::entity ent);
void animate_summon(Summon *sum, entt::entity ent);
void animate_pet(Pet *pet, entt::entity ent);
void animate_trap(Trap *trap, entt::entity ent);
void animate_worldmap();
void animate_cursor();