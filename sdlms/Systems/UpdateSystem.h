#pragma once
#include "Systems/System.h"
#include "Components/AnimatedSprite.h"
#include "Components/Transform.h"
#include "Components/HVMove.h"
#include "Components/Avatar.h"
#include "Components/Player.h"
#include "Components/Task.h"
#include "Components/Video.h"

#include "Entities/Portal.h"


class UpdateSystem : public System
{
public:
	void run(World &world) override;

private:
	bool update_animated_sprite(AnimatedSprite *aspr, World &world);
	void update_avatar(Avatar *hvm, Transform *tr, World &world);
	void update_portal(Portal *por, Transform *tr, World &world); // 三段式传送门
	bool update_task(Task *tas,World &world); // 三段式传送门
	static int update_video(void *vid); // 三段式传送门
};
