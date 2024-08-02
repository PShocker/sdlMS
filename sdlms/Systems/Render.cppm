module;

export module systems:render;

import components;

export void render_run();

void render_sprite(Transform *tr, SpriteWarp *spr);
void render_animated_sprite(Transform *tr, AnimatedSprite *a);
void render_back_sprite(Transform *tr, BackGround *bspr);
void render_character(const Transform *tr, Character *cha);
void render_afterimage(Transform *tr, AfterImage *aim, Character *cha);
void render_skill(Transform *tr, PlayerSkill *ski);
void render_portal(Transform *tr, Portal *por);
void render_npc(Transform *tr, Npc *npc);
void render_mob(Transform *tr, Mob *mob);
void render_hit(Transform *tr, HitEffect *hit);
