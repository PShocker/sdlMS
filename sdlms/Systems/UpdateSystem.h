#pragma once
#include "Systems/System.h"
#include "Components/AnimatedSprite.h"
#include "Components/Transform.h"
#include "Components/HVMove.h"
#include "Components/Avatar.h"
#include "Components/Player.h"

#include "Entities/Portal.h"

class UpdateSystem : public System
{
public:
	void run(World &world) override;

private:
	bool update_animated_sprite(AnimatedSprite *aspr, World &world);
	void update_avatar(Avatar *hvm, Transform *tr, World &world);
	void update_portal(Portal *por, Transform *tr, World &world); // 三段式传送门
};
