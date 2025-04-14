#pragma once

#include <SDL3/SDL.h>
#include <optional>

#include "Components/Components.h"
#include "UI/UI.h"

void render_run();

void render_texture(SDL_Texture *texture, SDL_FRect *src_rect, SDL_FRect *pos_rect, int alpha, int flip = 0, float rotation = 0, SDL_FPoint *origin = nullptr);
void render_sprite(SDL_FPoint &p, Sprite::Wrap *sprw, int flip = 0, float rotation = 0, SDL_FPoint *origin = nullptr, int alpha = 255);
void render_sprite(Transform *tr, Sprite::Wrap *sprw, SDL_FPoint *origin = nullptr, int alpha = 255);
void render_animated_sprite(SDL_FPoint &p, AnimatedSprite *a, int alpha = 255);
void render_animated_sprite(Transform *tr, AnimatedSprite *a, SDL_FPoint *origin = nullptr, int alpha = 255);
void render_back_sprite(Transform *tr, BackGround *bspr);
void render_character(const Transform *tr, Character *cha, int invincible);
void render_afterimage(Transform *tr, AfterImage *aft, Character *cha);
void render_effect_front(Transform *tr, Effect *eff);
void render_effect(Transform *tr, Effect *eff);
void render_effect_back(Transform *tr, Effect *eff);
void render_portal(Transform *tr, Portal *por);
void render_npc(Transform *tr, Npc *npc);
void render_mob(Transform *tr, Mob *mob);
void render_damage();
void render_tomb(Tomb *tomb);
void render_drop(Transform *tr, Drop *dro);
void render_reactor(Transform *tr, Reactor *r);
void render_install(Transform *tr, Install *i);
void render_summon(Transform *tr, Summon *sum);
void render_pet(Transform *tr, Pet *pet);
void render_nametag(Transform *tr, NameTag *nametag);
void render_chatballoon();
void render_uibuff();
void render_statusbar();
void render_worldmap();
void render_keyconfig();
void render_uiitem();
void render_uiskill();
void render_uistat();
void render_minimap();
void render_uiequip();
void render_cursor();
void render_tooltip();
void render_gaintip();
void render_quickslot();
void render_uinotice();
void render_ui();