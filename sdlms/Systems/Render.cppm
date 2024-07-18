module;

export module systems:render;

import components;

export void render_run();

void render_sprite(Transform *tr, Sprite *spr);
void render_animated_sprite(Transform *tr, AnimatedSprite *aspr);
void render_back_sprite(Transform *tr, BackGround *bspr);
void render_character(Transform *tr, Character *cha);
