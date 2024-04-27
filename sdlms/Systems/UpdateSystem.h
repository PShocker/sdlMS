#pragma once
#include "Systems/System.h"
#include "Components/AnimatedSprite.h"

class UpdateSystem : public System
{
public:
	void run(World &world) override;

private:
	bool update_animated_sprite(AnimatedSprite *aspr, World &world);
};
