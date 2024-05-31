#pragma once
#include "Systems/System.h"
#include "Components/Transform.h"
#include "Components/Camera.h"

class CameraSystem : public System
{
public:
	void run(World &world) override;

private:
	void update_camera(Camera *cam, Transform *tr, World &world);
};
