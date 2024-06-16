#pragma once
#include "Systems/System.h"
#include "Components/Transform.h"
#include "Components/Physic/Normal.h"
#include "Components/Player.h"
#include "Components/RandomInput.h"

class InputSystem : public System
{
public:
	void run(World &world) override;

private:
	void update_player(Player *nor, World &world);
	void update_random_input(RandomInput *ran, World &world);
};
