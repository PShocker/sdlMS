
module;

#include "../Components/Sprite.h"
#include "../Components/Transform.h"
#include "../Components/AnimatedSprite.h"
#include "../Components/BackSprite.h"

export module systems:animate;

export struct Animate
{
	static void run();
	static void animate_sprite(AnimatedSprite *aspr);
};
