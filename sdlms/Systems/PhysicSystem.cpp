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
			auto [x1, y1] = ri->get_m();
			auto [x2, y2] = ri->get_n();

			// 判断x碰撞
			bool is_collide_x = tr->get_position().x == std::clamp(tr->get_position().x, x1, x2) &&
								pos.x == std::clamp(pos.x, x1, x2);

			// 判断y碰撞
			bool is_collide_y = tr->get_position().y <= std::min(y1, y2) && pos.y >= std::max(y1, y2);

			if (is_collide_x && is_collide_y)
			{
				pos.y = ri->get_y(pos.x).value();
				nor->vspeed = 0;
				nor->type = Normal::GROUND;
				break;
			}
		}
	}
	tr->set_position(pos);
}
