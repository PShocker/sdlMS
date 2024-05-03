#pragma once
#include "Systems/System.h"
#include "Components/Transform.h"
#include "Components/Physic/Normal.h"

class PhysicSystem : public System
{
public:
	void run(World &world) override;

private:
	void update_nor(Normal *nor, World &world);
};
