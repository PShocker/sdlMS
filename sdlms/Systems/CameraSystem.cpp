#include "CameraSystem.h"
#include "Entities/Border.h"
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

	auto hdelta = tr->get_position().x - cam->get_w() / 2 - cam->get_x();
	if (std::abs(hdelta) >= 5.0)
	{
		cam->set_x(cam->get_x() + hdelta * (12.0 / cam->get_w()));
	}

	auto vdelta = tr->get_position().y - cam->get_h() / 2 - cam->get_y();
	if (std::abs(vdelta) >= 5.0)
	{
		cam->set_y(cam->get_y() + vdelta * (12.0 / cam->get_h()));
	}

	if (world.get_entity_map().contains(typeid(Border)))
	{
		auto border = world.get_entitys<Border>().find(0)->second;
		float left = border->get_left();
		float right = border->get_right();
		float top = border->get_top();
		float bottom = border->get_bottom();

		if (cam->get_x() < left)
		{
			cam->set_x(left);
		}
		else if (cam->get_x() + cam->get_w() > right)
		{
			cam->set_x(right - cam->get_w());
		}

		if (cam->get_y() + cam->get_h() > bottom)
		{
			cam->set_y(bottom - cam->get_h());
		}
	}
}
