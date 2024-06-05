#pragma once
#include "Systems/System.h"
#include "Components/Sprite.h"
#include "Components/AnimatedSprite.h"
#include "Components/Transform.h"
#include "Components/HVTile.h"
#include "Components/HVMove.h"
#include "Components/Avatar.h"
#include "Components/Video.h"
#include "Entities/Npc.h"
#include "Entities/FootHold.h"
#include "Components/Line.h"

class RenderSystem : public System
{
public:
	void run(World &world) override;

private:
	void render_sprite(Transform *tr, Sprite *spr, World &world);
	void render_animated_sprite(Transform *tr, AnimatedSprite *aspr, World &world);
	void render_hvtile_sprite(Transform *tr, HVTile *hvt, World &world);
	void render_avatar_sprite(Transform *tr, Avatar *ava, World &world);
	void render_video(Transform *tr,Video *vid, World &world);
	void render_fh(FootHold *fh,World &world);
};
