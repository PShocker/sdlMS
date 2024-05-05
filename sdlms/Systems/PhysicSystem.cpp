#include "PhysicSystem.h"
#include "Entities/FootHold.h"
#include "Components/RigidLine.h"

void PhysicSystem::run(World &world)
{
	if (world.components_exist_of_type<Normal>())
	{
		for (auto &[index, nor] : world.get_components<Normal>())
		{
			update_nor(nor, world);
		}
	}
}

void PhysicSystem::update_nor(Normal *nor, World &world)
{
	auto tr = nor->get_owner_component<Transform>();
	float delta_time = world.delta_time() / 1000.0;
	auto friction = 800;
	// 计算出不同状态的速度
	switch (nor->type)
	{
	case Normal::GROUND:
		// 地面移动判断
		//  摩擦力
		if (nor->hspeed > 0)
		{
			nor->hforce -= friction;
			nor->hforce = std::max(-nor->hspeed / delta_time, nor->hforce);
		}
		else if (nor->hspeed < 0)
		{
			nor->hforce += friction;
			nor->hforce = std::min(-nor->hspeed / delta_time, nor->hforce);
		}

		nor->hspeed += delta_time * nor->hforce;
		nor->hspeed = std::clamp(nor->hspeed, -125.0f, 125.0f);

		break;
	case Normal::AIR:
		// 默认重力为2000
		nor->vspeed += delta_time * 2000;
		nor->vspeed = std::min(nor->vspeed, 670.0f);
		break;
	case Normal::CLIMB:
		break;
	}

	auto d_x = nor->hspeed * delta_time;
	auto d_y = nor->vspeed * delta_time;

	auto pos = tr->get_position() + SDL_FPoint{(float)d_x, (float)d_y};

	if (nor->vspeed > 0)
	{
		for (auto &[id, fh] : world.get_entitys<FootHold>())
		{
			auto ri = fh->get_component<RigidLine>();
			auto collide = intersect(tr->get_position(), pos, ri->get_m(), ri->get_n());

			if (collide.has_value())
			{
				pos = collide.value();
				nor->vspeed = 0;
				nor->type = Normal::GROUND;
				break;
			}
		}
	}
	tr->set_position(pos);
}

std::optional<SDL_FPoint> PhysicSystem::intersect(SDL_FPoint p1, SDL_FPoint p2, SDL_FPoint p3, SDL_FPoint p4)
{
	std::optional<SDL_FPoint> p;
	// 快速排斥实验
	if ((p1.x > p2.x ? p1.x : p2.x) < (p3.x < p4.x ? p3.x : p4.x) ||
		(p1.y > p2.y ? p1.y : p2.y) < (p3.y < p4.y ? p3.y : p4.y) ||
		(p3.x > p4.x ? p3.x : p4.x) < (p1.x < p2.x ? p1.x : p2.x) ||
		(p3.y > p4.y ? p3.y : p4.y) < (p1.y < p2.y ? p1.y : p2.y))
	{
		return p;
	}
	// 跨立实验
	if ((((p1.x - p3.x) * (p4.y - p3.y) - (p1.y - p3.y) * (p4.x - p3.x)) *
		 ((p2.x - p3.x) * (p4.y - p3.y) - (p2.y - p3.y) * (p4.x - p3.x))) > 0 ||
		(((p3.x - p1.x) * (p2.y - p1.y) - (p3.y - p1.y) * (p2.x - p1.x)) *
		 ((p4.x - p1.x) * (p2.y - p1.y) - (p4.y - p1.y) * (p2.x - p1.x))) > 0)
	{
		return p;
	}

	auto x = ((p1.y - p3.y) * (p2.x - p1.x) * (p4.x - p3.x) + p3.x * (p4.y - p3.y) * (p2.x - p1.x) - p1.x * (p2.y - p1.y) * (p4.x - p3.x)) / ((p4.x - p3.x) * (p1.y - p2.y) - (p2.x - p1.x) * (p3.y - p4.y));
	auto y = (p2.y * (p1.x - p2.x) * (p4.y - p3.y) + (p4.x - p2.x) * (p4.y - p3.y) * (p1.y - p2.y) - p4.y * (p3.x - p4.x) * (p2.y - p1.y)) / ((p1.x - p2.x) * (p4.y - p3.y) - (p2.y - p1.y) * (p3.x - p4.x));

	p = {x, y};

	return p;
}
