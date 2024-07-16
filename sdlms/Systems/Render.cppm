module;

export module systems:render;

import components;

export struct Render
{
	static void run();
	static void render_sprite(Transform *tr, Sprite *spr);
	static void render_animated_sprite(Transform *tr, AnimatedSprite *aspr);
	static void render_back_sprite(Transform *tr, BackGround *bspr);
};
