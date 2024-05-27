#pragma once
#include "Systems/System.h"
#include "Components/DistanceSprite.h"
#include "Components/Transform.h"

class SpriteSystem : public System
{
public:
	void run(World &world) override;

private:
	void update_dis(DistanceSprite *dis, World &world);
};
