#pragma once
#include "Systems/System.h"
#include "Entities/Portal.h"
#include "Entities/Npc.h"
#include "Components/Transform.h"

class EntitySystem : public System
{
public:
	void run(World &world) override;

private:
	void update_portal(Portal *por, World &world);
	void update_npc(Npc *npc, World &world);
};
