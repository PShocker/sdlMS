module;

export module systems:render;

import components;

export void render_run();

void render_sprite(Transform *tr, Sprite *spr);
void render_animated_sprite(Transform *tr, Animated *a);
void render_back_sprite(Transform *tr, BackGround *bspr);
void render_character(const Transform *tr, Character *cha);
void render_afterimage(Transform *tr, AfterImage *aim, Character *cha);
void render_skill(Transform *tr, Skill *ski);
void render_portal(Transform *tr, Portal *por);
