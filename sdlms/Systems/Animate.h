#pragma once
#include "../Components/Sprite.h"
#include "../Components/Transform.h"
#include "../Components/AnimatedSprite.h"
#include "../Components/BackSprite.h"

struct Animate
{
	static void run();
	static void animate_sprite(AnimatedSprite *aspr);
};
