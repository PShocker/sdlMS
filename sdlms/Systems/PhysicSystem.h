#pragma once
#include "Systems/System.h"
#include "Components/Transform.h"
#include "Components/Physic/Normal.h"
#include <optional>
#include <ctime>

class PhysicSystem : public System
{
public:
	PhysicSystem(){};
	~PhysicSystem() = default;
	void run(World &world) override;

private:
	void update_normal(Normal *nor, World &world);
	bool want_climb(Transform *tr, Normal *nor, World &world);
	bool want_prone(Normal *nor, World &world);
	bool want_fall(Transform *tr, Normal *nor, World &world);
	bool want_stand(Normal *nor, World &world);
	bool want_jump(Transform *tr, Normal *nor, World &world);
	bool want_attack(Transform *tr, Normal *nor, World &world);
	bool want_portal(Transform *tr, Normal *nor, World &world);
	bool walk(Transform *tr, Normal *nor, World &world, float delta_time);
	void fall(Transform *tr, Normal *nor, float delta_time, World &world);
	void climb(Transform *tr, Normal *nor, float delta_time);
	inline std::optional<SDL_FPoint> intersect(SDL_FPoint p1,
											   SDL_FPoint p2,
											   SDL_FPoint p3,
											   SDL_FPoint p4);
};
