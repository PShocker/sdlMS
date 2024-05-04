#include "CameraSystem.h"
#include <SDL2/SDL.h>

void CameraSystem::run(World &world)
{
	if (world.components_exist_of_type<Camera>())
	{
		auto camera = world.get_components<Camera>().find(0)->second;
		update_camera(camera, camera->get_owner_component<Transform>(), world);
	}
}

void CameraSystem::update_camera(Camera *cam, Transform *tr, World &world)
{
	cam->set_x(tr->get_position().x - cam->get_w() / 2);
	cam->set_y(tr->get_position().y - cam->get_h() / 2);
}
