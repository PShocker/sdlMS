#pragma once
#include "Systems/System.h"
#include "Components/AnimatedSprite.h"
#include "Components/Transform.h"
#include "Components/HVMove.h"
#include "Components/Avatar.h"
#include "Components/Video.h"

#include "Entities/Timer.h"

class DeltaTimeSystem : public System
{
public:
	void run(World &world) override;

private:
	bool update_animated_sprite(AnimatedSprite *aspr, int delta_time, World &world);
	void update_avatar(Avatar *hvm, int delta_time, World &world);
	void update_timer(int delta_time, World &world);
};
