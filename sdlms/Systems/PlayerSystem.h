#pragma once
#include "Systems/System.h"
#include "Components/Transform.h"
#include "Components/Physic/Normal.h"
#include "Components/Player.h"

class PlayerSystem : public System
{
public:
	void run(World &world) override;

private:
	void update_player(Player *nor, World &world);
};
