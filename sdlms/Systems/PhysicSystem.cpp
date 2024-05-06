#include "PhysicSystem.h"
#include "Entities/FootHold.h"
#include "Entities/LadderRope.h"
#include "Entities/Character.h"
#include "Components/RigidLine.h"
#include "Components/CrawlLine.h"
#include "Components/Avatar.h"

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
	// 计算出不同状态的速度
	switch (nor->type)
	{
	case Normal::Ground:
		// 首先判断是否爬梯子
		if (want_climb(tr, nor, world))
		{
			return;
		}
		// 地面移动判断
		//  摩擦力
		walk(tr, nor, world, delta_time);
		break;
	case Normal::Air:
		fall(tr, nor, delta_time, world);
		break;
	case Normal::Climb:
		climb(tr, nor);
		break;
	}

	// auto d_x = nor->hspeed * delta_time;
	// auto d_y = nor->vspeed * delta_time;

	// auto new_pos = tr->get_position() + SDL_FPoint{(float)d_x, (float)d_y};

	// if (nor->vspeed > 0)
	// {
	// 	for (auto &[id, fh] : world.get_entitys<FootHold>())
	// 	{
	// 		auto ri = fh->get_component<RigidLine>();
	// 		auto collide = intersect(tr->get_position(), new_pos, ri->get_m(), ri->get_n());

	// 		if (collide.has_value())
	// 		{
	// 			new_pos = collide.value();
	// 			nor->vspeed = 0;
	// 			nor->type = Normal::Ground;
	// 			break;
	// 		}
	// 	}
	// }
	// tr->set_position(new_pos);
}

bool PhysicSystem::want_climb(Transform *tr, Normal *nor, World &world)
{
	if (nor->want_climb)
	{
		auto pos = tr->get_position();
		LadderRope *lad = nullptr;
		for (auto &[id, lr] : world.get_entitys<LadderRope>())
		{
			auto cl = lr->get_component<CrawlLine>();

			// 判断x坐标是否在梯子范围内
			if (pos.x = std::clamp(pos.x,
								   std::min(cl->get_m().x, cl->get_n().x) - 10,
								   std::max(cl->get_m().x, cl->get_n().x) + 10))
			{
				if (nor->want_climb == Normal::Up)
				{
					// 向上爬
					if (pos.y == std::clamp(pos.y,
											std::min(cl->get_m().y, cl->get_n().y),
											std::max(cl->get_m().y, cl->get_n().y) + 5))
					{
						// 垂直的线
						lad = lr;
						break;
					}
				}
				else if (nor->want_climb == Normal::Down && nor->type == Normal::Ground)
				{
					// 向下爬
					if (std::min(cl->get_m().y, cl->get_n().y) - pos.x <= 5)
					{
						lad = lr;
						break;
					}
				}
			}
		}
		if (lad != nullptr)
		{
			// 爬到了梯子上
			auto cl = lad->get_component<CrawlLine>();
			tr->set_position(SDL_FPoint{cl->get_m().x, cl->get_min_y()});

			if (nor->get_owner_component<Avatar>() != nullptr)
			{
				auto ava = nor->get_owner_component<Avatar>();
				// 判断爬的是梯子还是绳子
				if (lad->l == 1)
				{
					// 梯子
					ava->switch_act(Avatar::ACTION::LADDER);
				}
				else
				{
					// 绳子
					ava->switch_act(Avatar::ACTION::ROPE);
				}
				ava->animate = false;
			}
			nor->get_owner()->add_entity(lad);
			nor->vspeed = 0;
			nor->hspeed = 0;
			nor->type = Normal::Climb;
			return true;
		}
	}
	return false;
}

void PhysicSystem::walk(Transform *tr, Normal *nor, World &world, float delta_time)
{
	auto friction = 800;

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

	if (std::abs(nor->hspeed) < 0.1)
	{
		// 如果速度太小,则直接设置0,避免浮点数精度问题
		nor->hspeed = 0;
	}
	auto d_x = nor->hspeed * delta_time;
	auto x = d_x + tr->get_position().x;
	auto y = tr->get_position().y;

	auto fh = tr->get_owner()->get_entity<FootHold>();
	auto fhs = world.get_entitys<FootHold>();
	auto rl = fh->get_component<RigidLine>();

	// 切换fh
	// 往左走
	while (x < rl->get_min_x())
	{
		fh = tr->get_owner()->get_entity<FootHold>();
		rl = fh->get_component<RigidLine>();
		if (fhs.contains(fh->prev))
		{
			fh = fhs.find(fh->prev)->second;
		}
		else
		{
			// 从fh掉落
			nor->type = Normal::Air;
			break;
		}
		if (!rl->line->get_k().has_value())
		{
			if (y > rl->get_min_y())
			{
				// 撞墙,人物x不变
				x = tr->get_position().x;
				nor->hspeed = 0;
				break;
			}
			else
			{
				// 从fh掉落
				nor->type = Normal::Air;
				break;
			}
		}
		tr->get_owner()->add_entity(fh);
	}

	// 往右走
	while (x > rl->get_max_x())
	{
		fh = tr->get_owner()->get_entity<FootHold>();
		rl = fh->get_component<RigidLine>();

		if (fhs.contains(fh->next))
		{
			fh = fhs.find(fh->next)->second;
		}
		else
		{
			// 从fh掉落
			nor->type = Normal::Air;
			break;
		}
		if (!rl->line->get_k().has_value())
		{
			// 撞墙
			if (y > rl->get_min_y())
			{
				// 撞墙,人物x不变
				x = tr->get_position().x;
				nor->hspeed = 0;
				break;
			}
			else
			{
				// 从fh掉落
				nor->type = Normal::Air;
				break;
			}
		}
		tr->get_owner()->add_entity(fh);
	}

	if (rl->line->get_y(x).has_value())
	{
		tr->set_y(rl->line->get_y(x).value());
	}
	else
	{
		tr->set_y(y);
	}
	tr->set_x(x);
}

void PhysicSystem::fall(Transform *tr, Normal *nor, float delta_time, World &world)
{
	// 默认重力为2000
	nor->vspeed += delta_time * 2000;
	nor->vspeed = std::min(nor->vspeed, 670.0f);

	auto d_x = nor->hspeed * delta_time;
	auto d_y = nor->vspeed * delta_time;

	auto new_pos = tr->get_position() + SDL_FPoint{(float)d_x, (float)d_y};

	if (nor->vspeed > 0)
	{
		for (auto &[id, fh] : world.get_entitys<FootHold>())
		{
			auto rl = fh->get_component<RigidLine>();
			auto collide = intersect(tr->get_position(), new_pos, rl->get_m(), rl->get_n());

			if (collide.has_value())
			{
				new_pos = collide.value();
				nor->vspeed = 0;
				nor->type = Normal::Ground;
				nor->get_owner()->add_entity(fh);
				break;
			}
		}
	}
	tr->set_position(new_pos);
}

void PhysicSystem::climb(Transform *tr, Normal *nor)
{
	nor->hspeed = 0;
	// 判断是否脱离梯子
	auto cl = tr->get_owner_component<CrawlLine>();
	if (tr->get_position().y < cl->get_min_y())
	{
		tr->set_y(cl->get_min_y() - 5);
		nor->type = Normal::Air;
		nor->vspeed = 0;
	}
	if (tr->get_position().y > cl->get_max_y())
	{
		nor->type = Normal::Air;
		nor->vspeed = 0;
	}
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
