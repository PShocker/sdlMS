module;

#include "entt/entt.hpp"

export module systems:animate;

import components;

export void animate_run();

bool animate_sprite(Animated *a);
void animate_character(Character *cha);
void animate_afterimage(AfterImage *aim, Character *cha);
void animate_skill(Skill* ski);
void animate_portal(Portal* por);
void animate_npc(Npc* npc);