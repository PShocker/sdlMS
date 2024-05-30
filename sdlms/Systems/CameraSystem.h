#pragma once
#include "Systems/System.h"
#include "Components/Transform.h"
#include "Components/Camera.h"

class CameraSystem : public System
{
public:
	const int LIMIT_X = 400;
	const int LIMIT_Y = 120;
	void run(World &world) override;

private:
	void update_camera(Camera *cam, Transform *tr, World &world);
};
