#include <ranges>
#include <utility>
#include <algorithm>

#include "PhysicSystem.h"
#include "Entities/FootHold.h"
#include "Entities/LadderRope.h"
#include "Entities/Character.h"
#include "Components/RigidLine.h"
#include "Components/CrawlLine.h"
#include "Components/Avatar.h"
#include "Components/Task.h"
#include "Components/Player.h"

void PhysicSystem::run(World &world)
{
	if (world.components_exist_of_type<Normal>())
	{
		for (auto &[index, nor] : world.get_components<Normal>())
		{
			update_normal(nor, world);
		}
	}
}

void PhysicSystem::update_normal(Normal *nor, World &world)
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
		if (want_fall(tr, nor, world))
		{
			return;
		}
		if (want_prone(tr, nor, world))
		{
			return;
		}

		// 地面移动判断
		walk(tr, nor, world, delta_time);
		break;
	case Normal::Air:
		// 首先判断是否爬梯子
		if (want_climb(tr, nor, world))
		{
			return;
		}
		// 跳跃
		fall(tr, nor, delta_time, world);
		break;
	case Normal::Climb:
		// 爬梯子
		climb(tr, nor, delta_time);
		break;
	}
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
			if (pos.x == std::clamp(pos.x,
									cl->get_min_x() - 10,
									cl->get_max_x() + 10))
			{
				if (nor->want_climb == Normal::Up)
				{
					if (nor->type == Normal::Ground)
					{
						// 地面向上爬
						if (pos.y > cl->get_max_y() && pos.y <= cl->get_max_y() + 5)
						{
							// 垂直的线
							lad = lr;
							break;
						}
					}
					else if (nor->type == Normal::Air)
					{
						// 空中向上爬
						if (pos.y == std::clamp(pos.y,
												cl->get_min_y(),
												cl->get_max_y()))
						{
							// 垂直的线
							lad = lr;
							break;
						}
					}
				}
				else if (nor->want_climb == Normal::Down)
				{
					if (nor->type == Normal::Ground)
					{
						// 向下爬
						if (pos.y < cl->get_min_y() && pos.y >= cl->get_min_y() - 5)
						{
							lad = lr;
							tr->set_y(cl->get_min_y());
							break;
						}
					}
				}
			}
		}
		if (lad != nullptr)
		{
			// 爬到了梯子上
			auto cl = lad->get_component<CrawlLine>();

			tr->set_x(cl->get_m().x);

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
			// 修改人物z值
			world.destroy_component(tr, false);
			world.add_component(tr, lad->page * 30000 + 4000);

			nor->get_owner()->add_entity(lad);
			nor->vspeed = 0;
			nor->hspeed = 0;
			nor->type = Normal::Climb;
			return true;
		}
	}
	return false;
}

bool PhysicSystem::want_prone(Transform *tr, Normal *nor, World &world)
{
	if (nor->want_prone)
	{
		if (nor->get_owner_component<Avatar>() != nullptr)
		{
			auto ava = nor->get_owner_component<Avatar>();
			ava->switch_act(Avatar::ACTION::PRONE);
		}
		nor->hspeed = 0;
		return true;
	}
	return false;
}

bool PhysicSystem::want_fall(Transform *tr, Normal *nor, World &world)
{
	if (nor->want_fall)
	{
		auto foo = tr->get_owner()->get_entity<FootHold>();
		world.remove_entity(foo);
		world.add_entity(foo, -foo->id);

		// 添加定时任务到world
		auto callback = [](Entity *ent, World &world) -> void
		{
			auto foo = (FootHold *)ent;
			world.remove_entity(foo);
			world.add_entity(foo, foo->id);
			return;
		};
		Task *tas = new Task(callback, 300);
		foo->add_component(tas);
		world.add_component(tas);

		// 从fh掉落
		nor->type = Normal::Air;
		tr->get_owner()->remove_entity<FootHold>();
		if (nor->get_owner_component<Avatar>() != nullptr)
		{
			nor->get_owner_component<Avatar>()->switch_act(Avatar::ACTION::JUMP);
		}
		nor->vspeed = -250;
		// 修改人物z值
		world.destroy_component(tr, false);
		world.add_component(tr, 7 * 30000 + 4000);
		return true;
	}
	return false;
}

void PhysicSystem::walk(Transform *tr, Normal *nor, World &world, float delta_time)
{
	constexpr auto friction = 800;

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

	auto d_x = nor->hspeed * delta_time;
	auto x = d_x + tr->get_position().x;
	auto y = tr->get_position().y;

	auto &fhs = world.get_entitys<FootHold>();
	auto foo = tr->get_owner()->get_entity<FootHold>();
	auto rl = foo->get_component<RigidLine>();

	// 人物在fh移动的函数
	auto walk_fh = [&fhs, &foo, &nor, &rl, &x, &y, &tr](int next_fh) -> bool
	{
		FootHold *fh = nullptr; // 走到下一个fh
		if (fhs.contains(next_fh))
		{
			fh = fhs.find(next_fh)->second;
		}
		else
		{
			// 从fh掉落
			nor->type = Normal::Air;
			if (nor->get_owner_component<Avatar>() != nullptr)
			{
				nor->get_owner_component<Avatar>()->switch_act(Avatar::ACTION::JUMP);
			}
			nor->vspeed = 0;
			return false;
		}
		rl = fh->get_component<RigidLine>();
		if (!rl->line->get_k().has_value())
		{
			if (y > rl->get_min_y())
			{
				// 撞墙,人物x不变
				x = tr->get_position().x;
				nor->hspeed = 0;
				return false;
			}
			else
			{
				// 从fh掉落
				nor->type = Normal::Air;
				if (nor->get_owner_component<Avatar>() != nullptr)
				{
					nor->get_owner_component<Avatar>()->switch_act(Avatar::ACTION::JUMP);
				}
				nor->vspeed = 0;
				return false;
			}
		}
		foo = fh;
		return true;
	};
	// 往左走
	while (x < rl->get_min_x())
	{
		int next_fh = std::abs(foo->prev);
		if (walk_fh(next_fh) == false)
		{
			break;
		}
	}
	// 往右走
	while (x > rl->get_max_x())
	{
		int next_fh = std::abs(foo->next);
		if (walk_fh(next_fh) == false)
		{
			break;
		}
	}
	nor->get_owner()->add_entity(foo);
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

	// 下落
	if (nor->vspeed > 0)
	{
		for (auto &[id, fh] : world.get_entitys<FootHold>())
		{
			if (id < 0 && nor->get_owner_component<Player>() != nullptr)
			{
				// id<0表示下跳时的fh,需要排除玩家本身
				continue;
			}
			auto rl = fh->get_component<RigidLine>();
			auto collide = intersect(tr->get_position(), new_pos, rl->get_m(), rl->get_n());
			if (collide.has_value())
			{
				if (!rl->get_line()->get_k().has_value())
				{
					// 撞墙
					new_pos = tr->get_position();
					nor->hspeed = 0;
				}
				else
				{
					new_pos = collide.value();
					nor->type = Normal::Ground;
					nor->get_owner()->add_entity(fh);
					// 修改人物z值
					world.destroy_component(tr, false);
					world.add_component(tr, fh->page * 30000 + 4000);
				}
				break;
			}
		}
	}
	else if (nor->vspeed <= 0)
	{
		// 上升
		// 跳跃时只用考虑是否撞墙
		for (auto &[id, fh] : world.get_entitys<FootHold>())
		{
			auto rl = fh->get_component<RigidLine>();
			if (rl->get_n().x < rl->get_m().x && rl->get_n().y == rl->get_m().y)
			{
				// 天花板
				auto collide = intersect(tr->get_position(), new_pos, rl->get_m(), rl->get_n());
				if (collide.has_value())
				{
					new_pos = tr->get_position();
					nor->hspeed = 0;
					nor->vspeed = 0;
					break;
				}
			}
			if (!rl->get_line()->get_k().has_value())
			{
				// k值不存在,则为墙面,默认与所有墙面碰撞
				auto collide = intersect(tr->get_position(), new_pos, rl->get_m(), rl->get_n());
				if (collide.has_value())
				{
					new_pos.x = tr->get_position().x;
					nor->hspeed = 0;
					break;
				}
			}
			else
			{
				continue;
			}
		}
	}
	tr->set_position(new_pos);
}

void PhysicSystem::climb(Transform *tr, Normal *nor, float delta_time)
{
	// 判断是否脱离梯子
	auto cl = nor->get_owner()->get_entity<LadderRope>()->get_component<CrawlLine>();

	auto d_y = nor->vspeed * delta_time;
	auto y = d_y + tr->get_position().y;
	if (y < cl->get_min_y())
	{
		tr->set_y(cl->get_min_y() - 5);
		nor->type = Normal::Air;
		if (nor->get_owner_component<Avatar>() != nullptr)
		{
			nor->get_owner_component<Avatar>()->switch_act(Avatar::ACTION::JUMP);
		}
		nor->vspeed = 0;
	}
	else if (y > cl->get_max_y())
	{
		tr->set_y(cl->get_max_y());
		nor->type = Normal::Air;
		if (nor->get_owner_component<Avatar>() != nullptr)
		{
			nor->get_owner_component<Avatar>()->switch_act(Avatar::ACTION::JUMP);
		}
		nor->vspeed = 0;
	}
	else
	{
		if (nor->get_owner_component<Avatar>() != nullptr)
		{
			auto ava = nor->get_owner_component<Avatar>();
			if (nor->vspeed != 0)
			{
				ava->animate = true;
			}
			else
			{
				ava->animate = false;
			}
		}
		tr->set_y(y);
	}
}
// 线段相交法判断碰撞
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
