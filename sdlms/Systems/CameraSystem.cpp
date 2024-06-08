#include "CameraSystem.h"
#include "Entities/Border.h"
#include <cmath>

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
	auto h_prev_x = cam->get_x();							 // 上一帧摄像机坐标
	auto h_next_x = tr->get_position().x - cam->get_w() / 2; // 人物移动后新的摄像机坐标
	auto h_delta = h_next_x - h_prev_x;						 // 新的摄像机坐标与旧的摄像机坐标差值

	cam->set_x(std::lerp(h_prev_x, h_next_x, std::abs(h_delta) / 6000.0f));

	auto v_prev_y = cam->get_y();							 // 上一帧摄像机坐标
	auto v_next_y = tr->get_position().y - cam->get_h() / 2; // 人物移动后新的摄像机坐标
	auto v_delta = v_next_y - v_prev_y;						 // 新的摄像机坐标与旧的摄像机坐标差值

	cam->set_y(std::lerp(v_prev_y, v_next_y, std::abs(v_delta) / 6000.0f));

	if (world.entity_exist_of_type<Border>())
	{
		auto border = world.get_entitys<Border>().find(0)->second;
		auto left = border->get_left();
		auto right = border->get_right();
		auto top = border->get_top();
		auto bottom = border->get_bottom();

		if (left.has_value() && cam->get_x() < left.value())
		{
			cam->set_x(left.value());
		}
		else if (right.has_value() && cam->get_x() + cam->get_w() > right.value())
		{
			cam->set_x(right.value() - cam->get_w());
		}
		if (bottom.has_value() && cam->get_y() + cam->get_h() > bottom.value())
		{
			cam->set_y(bottom.value() - cam->get_h());
		}

		if (right.has_value() && left.has_value())
		{
			if (right.value() - left.value() < cam->get_w())
			{
				// 如果地图宽度小于摄像机宽度,地图居中显示
				cam->set_x(left.value() - (cam->get_w() - (right.value() - left.value())) / 2);
			}
		}
		if (bottom.has_value() && top.has_value())
		{
			if (bottom.value() - top.value() < cam->get_h())
			{
				// 如果地图高度小于摄像机高度,地图居中显示
				cam->set_y(top.value() - (cam->get_h() - (bottom.value() - top.value())) / 2);
			}
		}
	}
}
