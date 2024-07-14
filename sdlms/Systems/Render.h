#pragma once
#include "../Components/Sprite.h"
#include "../Components/Transform.h"
#include "../Components/AnimatedSprite.h"
#include "../Components/BackSprite.h"

struct Render
{
	static void run();
	static void render_sprite(Transform *tr, Sprite *spr);
	static void render_animated_sprite(Transform *tr, AnimatedSprite *aspr);
	static void render_back_sprite(Transform *tr, BackSprite *bspr);
};
