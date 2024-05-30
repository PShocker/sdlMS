#include "CameraSystem.h"
#include "Entities/Border.h"
#include "Core/MathHelper.h"
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

	MathHelper helper;
	auto hdelta = tr->get_position().x - cam->get_w() / 2 - cam->get_x();
	float next_x = 0;
	float next_y = 0;

	if (std::abs(hdelta) >= LIMIT_X)
	{
		next_x = helper.Lerp<float>(cam->get_x(), tr->get_position().x - cam->get_w() / 2, world.get_delta_time() * 6 / 1000.f);
		//cam->set_x(cam->get_x() + hdelta * (12.0 / cam->get_w()));
	}
	 else {
		next_x = helper.Lerp<float>(cam->get_x(), tr->get_position().x - cam->get_w() / 2, world.get_delta_time() * 2 / 1000.f);
	}

	auto vdelta = tr->get_position().y - cam->get_h() / 2 - cam->get_y();
	if (std::abs(vdelta) >= LIMIT_Y)
	{
		next_y = helper.Lerp<float>(cam->get_y(), tr->get_position().y - cam->get_h() / 2, world.get_delta_time() * 6 / 1000.f);
		//cam->set_y(cam->get_y() + vdelta * (12.0 / cam->get_h()));
	} 
	else {
		next_y = helper.Lerp<float>(cam->get_y(), tr->get_position().y - cam->get_h() / 2, world.get_delta_time() * 2 / 1000.f);
	}

	cam->set_x(next_x);
	cam->set_y(next_y);

	if (world.entity_exist_of_type<Border>())
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
