#pragma once
#include "Systems/System.h"
#include "Components/Transform.h"
#include "Components/Physic/Normal.h"
#include <optional>

class PhysicSystem : public System
{
public:
	void run(World &world) override;

private:
	void update_nor(Normal *nor, World &world);
	inline std::optional<SDL_FPoint> intersect(SDL_FPoint p1,
											   SDL_FPoint p2,
											   SDL_FPoint p3,
											   SDL_FPoint p4);
};
