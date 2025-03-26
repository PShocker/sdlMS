#pragma once

#include <SDL3/SDL.h>
#include <optional>

#include "Components/Components.h"
#include "UI/UI.h"

void render_run();

void render_sprite(SDL_FPoint &p, SpriteWarp *sprw, int flip = 0, float rotation = 0, SDL_FPoint *origin = nullptr);
void render_sprite(Transform *tr, SpriteWarp *sprw, SDL_FPoint *origin = nullptr);
void render_animated_sprite(SDL_FPoint &p, AnimatedSprite *a);
void render_animated_sprite(Transform *tr, AnimatedSprite *a, SDL_FPoint *origin = nullptr);
void render_back_sprite(Transform *tr, BackGround *bspr);
void render_character(const Transform *tr, Character *cha, int invincible);
void render_afterimage(Transform *tr, AfterImage *aft, Character *cha);
void render_effect_front(Transform *tr, Effect *eff);
void render_effect(Transform *tr, Effect *eff);
void render_effect_back(Transform *tr, Effect *eff);
void render_portal(Transform *tr, Portal *por);
void render_npc(Transform *tr, Npc *npc);
void render_mob(Transform *tr, Mob *mob);
void render_damage(Damage *dam);
void render_tomb(Tomb *tomb);
void render_drop(Transform *tr, Drop *dro);
void render_reactor(Transform *tr, Reactor *r);
void render_install(Transform *tr, Install *i);
void render_summon(Transform *tr, Summon *sum);
void render_pet(Transform *tr, Pet *pet);
void render_nametag(Transform *tr, NameTag *nametag);
void render_chatballoon(Transform *tr, Npc *npc, ChatBalloon *chatballoon);
void render_uibuff();
void render_statusbar();
void render_worldmap();
