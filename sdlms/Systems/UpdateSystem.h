#pragma once
#include "Systems/System.h"
#include "Components/AnimatedSprite.h"
#include "Components/Transform.h"
#include "Components/HVMove.h"

class UpdateSystem : public System
{
public:
	void run(World &world) override;

private:
	bool update_animated_sprite(AnimatedSprite *aspr, World &world);
	void update_hvmove(HVMove *hvm, Transform *tr, World &world);
};
