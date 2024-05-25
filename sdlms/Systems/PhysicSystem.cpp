#include <ranges>
#include <utility>
#include <algorithm>

#include "PhysicSystem.h"
#include "Entities/FootHold.h"
#include "Entities/LadderRope.h"
#include "Entities/Character.h"
#include "Entities/Mob.h"
#include "Entities/Portal.h"
#include "Components/RigidLine.h"
#include "Components/CrawlLine.h"
#include "Components/Avatar.h"
#include "Components/Player.h"
#include "Components/Camera.h"

#include "Core/Map.h"

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
	float delta_time = world.get_delta_time() / 1000.0;

	// 首先修改人物朝向
	if (nor->hkey == Normal::Right)
	{
		tr->set_flip(1);
	}
	else if (nor->hkey == Normal::Left)
	{
		tr->set_flip(0);
	}
	// 计算出不同状态的速度
	switch (nor->type)
	{
	[[likely]]
	case Normal::Ground:
		// 判断是否进传送门
		[[unlikely]]
		if (want_portal(tr, nor, world))
		{
			return;
		}
		// 判断是否爬梯子
		[[unlikely]]
		if (want_climb(tr, nor, world))
		{
			return;
		}
		// 下跳
		[[unlikely]]
		if (want_fall(tr, nor, world))
		{
			return;
		}
		// 趴
		[[unlikely]]
		if (want_prone(nor, world))
		{
			return;
		}
		// 跳
		[[unlikely]]
		if (want_jump(tr, nor, world))
		{
			return;
		}
		// 地面移动判断
		[[unlikely]]
		if (!walk(tr, nor, world, delta_time))
		{
			return;
		}
		[[likely]]
		if (want_stand(nor, world))
		{
			return;
		}
		break;
	case Normal::Air:
		// 首先判断是否爬梯子
		[[unlikely]]
		if (want_climb(tr, nor, world))
		{
			return;
		}
		// 空中
		fall(tr, nor, delta_time, world);
		break;
	case Normal::Climb:
		// 爬梯子
		[[unlikely]]
		if (want_jump(tr, nor, world))
		{
			return;
		}
		climb(tr, nor, delta_time);
		break;
	}
}

bool PhysicSystem::want_climb(Transform *tr, Normal *nor, World &world)
{
	if (nor->vkey != Normal::None)
	{
		auto pos = tr->get_position();
		LadderRope *lad = nullptr;
		for (auto &[id, lr] : world.get_entitys<LadderRope>())
		{
			auto cl = lr->get_component<CrawlLine>();

			// 判断x坐标是否在梯子范围内
			if (pos.x >= cl->get_min_x() - 10 && pos.x <= cl->get_max_x() + 10)
			{
				if (nor->vkey == Normal::Up)
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
						if (pos.y > cl->get_min_y() && pos.y < cl->get_max_y())
						{
							// 垂直的线
							lad = lr;
							break;
						}
					}
				}
				else if (nor->vkey == Normal::Down)
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

			nor->get_owner()->add_entity(lad, 0);
			nor->vspeed = 0;
			nor->hspeed = 0;
			nor->type = Normal::Climb;
			return true;
		}
	}
	return false;
}

bool PhysicSystem::want_prone(Normal *nor, World &world)
{
	if (nor->vkey == Normal::Down)
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
	if (nor->vkey == Normal::Down && nor->lalt)
	{
		auto foo = tr->get_owner()->get_entity<FootHold>(0);
		world.remove_entity(foo);
		world.add_entity(foo, -foo->id);

		// 添加定时任务到world
		auto callback = [](Uint32 interval, void *param) -> Uint32
		{
			auto world = World::get_world();
			auto foo = (FootHold *)param;
			world->remove_entity(foo);
			world->add_entity(foo, foo->id);
			return 0;
		};
		SDL_TimerID timerID = SDL_AddTimer(200, callback, foo); // 定时器

		// 从fh掉落
		nor->type = Normal::Air;
		if (nor->get_owner_component<Avatar>() != nullptr)
		{
			nor->get_owner_component<Avatar>()->switch_act(Avatar::ACTION::JUMP);
		}
		else if (nor->get_owner<Mob>() != nullptr)
		{
			nor->get_owner<Mob>()->switch_act(u"jump");
		}
		nor->vspeed = -140;
		// 修改人物z值
		world.destroy_component(tr, false);
		world.add_component(tr, 7 * 30000 + 4000);
		return true;
	}
	return false;
}

bool PhysicSystem::want_stand(Normal *nor, World &world)
{
	if (nor->hkey == Normal::None)
	{
		// 还需要进行alert状态判断
		if (nor->get_owner_component<Avatar>() != nullptr)
		{
			auto ava = nor->get_owner_component<Avatar>();
			ava->switch_act(Avatar::ACTION::STAND1);
		}
		else if (nor->get_owner<Mob>() != nullptr)
		{
			nor->get_owner<Mob>()->switch_act(u"stand");
		}
		return true;
	}
	return false;
}

bool PhysicSystem::want_jump(Transform *tr, Normal *nor, World &world)
{
	if (nor->lalt)
	{
		if (nor->type == Normal::Ground)
		{
			nor->type = Normal::Air;
			nor->vspeed = -555;

			if (nor->get_owner_component<Avatar>() != nullptr)
			{
				// 修改纸娃娃状态
				nor->get_owner_component<Avatar>()->switch_act(Avatar::ACTION::JUMP);
			}
			else if (nor->get_owner<Mob>() != nullptr)
			{
				nor->get_owner<Mob>()->switch_act(u"jump");
			}
			return true;
		}
		else if (nor->type == Normal::Climb && (nor->hkey == Normal::Right || nor->hkey == Normal::Left) && (nor->vkey != Normal::Up))
		{
			nor->vspeed = -300;
			nor->type = Normal::Air;
			if (nor->hkey == Normal::Right)
			{
				nor->hspeed = 100;
			}
			else if (nor->hkey == Normal::Left)
			{
				nor->hspeed = -100;
			}

			if (nor->get_owner_component<Avatar>() != nullptr)
			{
				// 修改纸娃娃状态
				nor->get_owner_component<Avatar>()->switch_act(Avatar::ACTION::JUMP);
				nor->get_owner_component<Avatar>()->animate = true;
			}
			else if (nor->get_owner<Mob>() != nullptr)
			{
				nor->get_owner<Mob>()->switch_act(u"jump");
			}
			return true;
		}
	}
	return false;
}

bool PhysicSystem::want_portal(Transform *tr, Normal *nor, World &world)
{
	if (tr->get_owner_component<Player>() != nullptr)
	{
		if (nor->vkey == Normal::Up)
		{
			for (auto &[id, por] : world.get_entitys<Portal>())
			{
				auto pla_pos = tr->get_position();
				auto por_pos = por->get_component<Transform>();
				auto por_spr = por->get_component<AnimatedSprite>();
				auto tn = por->tn;
				Sprite *spr = nullptr;
				if (por->get_component<Sprite>() != nullptr)
				{
					spr = por->get_component<Sprite>();
				}
				else if (por->get_component<AnimatedSprite>() != nullptr)
				{
					spr = por->get_component<AnimatedSprite>()->get_current_sprite();
				}
				if (spr != nullptr)
				{
					auto rect = SDL_FRect{por_pos->get_position().x - spr->origin.x,
										  por_pos->get_position().y - spr->origin.y,
										  (float)spr->get_width(),
										  (float)spr->get_height()};
					if (SDL_PointInFRect(&pla_pos, &rect))
					{
						// 切换地图
						Map::load(por->tm, &world);
						world.tick_delta_time();
						// 切换人物坐标
						for (auto &[id, p] : world.get_entitys<Portal>())
						{
							if (tn == p->pn)
							{
								tr->set_position(p->get_component<Transform>()->get_position() + SDL_FPoint{0, -50});
								// 调整相机位置
								auto camera = world.get_components<Camera>().find(0)->second;
								camera->set_x(tr->get_position().x - camera->get_w() / 2);
								camera->set_y(tr->get_position().y - camera->get_h() / 2);
								break;
							}
						}
						nor->type = Normal::Air;
						nor->vspeed = 0;
						nor->hspeed = 0;
						return true;
					}
				}
			}
		}
	}
	return false;
}

bool PhysicSystem::walk(Transform *tr, Normal *nor, World &world, float delta_time)
{
	if (nor->hkey != Normal::None)
	{
		if (nor->hkey == Normal::Right)
		{
			nor->hforce = 1400;
		}
		else if (nor->hkey == Normal::Left)
		{
			nor->hforce = -1400;
		}
		if (nor->get_owner_component<Avatar>() != nullptr)
		{
			nor->get_owner_component<Avatar>()->switch_act(Avatar::ACTION::WALK1);
		}
		else if (nor->get_owner<Mob>() != nullptr)
		{
			nor->get_owner<Mob>()->switch_act(u"move");
		}
	}
	else
	{
		nor->hforce = 0;
	}

	constexpr auto friction = 800; // 摩擦力

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
	auto foo = tr->get_owner()->get_entity<FootHold>(0);
	auto rl = foo->get_component<RigidLine>();

	// 人物在fh移动的函数
	auto walk_fh = [&fhs, &foo, &nor, &x, &y, &tr](int next_fh) -> bool
	{
		FootHold *fh = nullptr; // 走到下一个fh
		[[likely]]
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
			tr->set_y(y);
			tr->set_x(x);
			return false;
		}
		auto rl = fh->get_component<RigidLine>();
		if (!rl->line->get_k().has_value())
		{
			if (y == rl->get_max_y())
			{
				// 撞墙
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
				tr->set_y(y);
				tr->set_x(x);
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
			// 从fh掉落,撞墙
			return false;
		}
		rl = foo->get_component<RigidLine>();
	}
	// 往右走
	while (x > rl->get_max_x())
	{
		int next_fh = std::abs(foo->next);
		if (walk_fh(next_fh) == false)
		{
			// 从fh掉落,撞墙
			return false;
		}
		rl = foo->get_component<RigidLine>();
	}
	// 地面上
	nor->get_owner()->add_entity(foo, 0);
	tr->set_y(rl->line->get_y(x).value());
	tr->set_x(x);
	return true;
}

void PhysicSystem::fall(Transform *tr, Normal *nor, float delta_time, World &world)
{
	if (nor->hkey == Normal::Right)
	{
		nor->hspeed += 1;
	}
	else if (nor->hkey == Normal::Left)
	{
		nor->hspeed -= 1;
	}
	// 默认重力为2000
	nor->vspeed += delta_time * 2000;
	nor->vspeed = std::min(nor->vspeed, 670.0f);

	auto d_x = nor->hspeed * delta_time;
	auto d_y = nor->vspeed * delta_time;

	auto new_pos = tr->get_position() + SDL_FPoint{(float)d_x, (float)d_y};

	// 人物之前的fh
	auto foo = tr->get_owner()->get_entity<FootHold>(0);

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
					// k值不存在,则为墙面,且必须和同一级的墙碰撞
					if (foo == nullptr || foo->get_page() == fh->get_page())
					{
						// 判断墙面碰撞方向
						if ((nor->hspeed > 0 && rl->get_m().y > rl->get_n().y) || (nor->hspeed < 0 && rl->get_m().y < rl->get_n().y))
						{
							new_pos.x = tr->get_position().x;
							new_pos.y = std::min(rl->get_max_y(), new_pos.y);
							nor->hspeed = 0;
						}
					}
				}
				else
				{
					// 落地
					new_pos.x = std::clamp(collide.value().x, rl->get_min_x(), rl->get_max_x());
					new_pos.y = rl->get_line()->get_y(new_pos.x).value();
					nor->type = Normal::Ground;
					// 修改速度
					if ((nor->hkey == Normal::Right && nor->hspeed > 0) || (nor->hkey == Normal::Left && nor->hspeed < 0))
					{
						nor->hspeed /= 2;
					}
					else
					{
						nor->hspeed /= 3;
					}
					nor->get_owner()->add_entity(fh, 0);
					// 修改人物z值
					world.destroy_component(tr, false);
					world.add_component(tr, fh->page * 30000 + 4000);
					break;
				}
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
				// k值不存在,则为墙面,且必须和同一级的墙碰撞
				if (foo == nullptr || foo->get_page() == fh->get_page())
				{
					// 判断墙面碰撞方向
					if ((nor->hspeed > 0 && rl->get_m().y > rl->get_n().y) || (nor->hspeed < 0 && rl->get_m().y < rl->get_n().y))
					{
						auto collide = intersect(tr->get_position(), new_pos, rl->get_m(), rl->get_n());
						if (collide.has_value())
						{
							new_pos.x = tr->get_position().x;
							nor->hspeed = 0;
							break;
						}
					}
				}
			}
		}
	}
	tr->set_position(new_pos);
}

void PhysicSystem::climb(Transform *tr, Normal *nor, float delta_time)
{
	if (nor->vkey == Normal::Up)
	{
		nor->vspeed = -100;
	}
	else if (nor->vkey == Normal::Down)
	{
		nor->vspeed = 100;
	}
	else
	{
		nor->vspeed = 0;
	}

	// 判断是否脱离梯子
	auto lr = nor->get_owner()->get_entity<LadderRope>(0);
	auto cl = lr->get_component<CrawlLine>();

	auto d_y = nor->vspeed * delta_time;
	auto y = d_y + tr->get_position().y;
	if (y < cl->get_min_y())
	{
		if (lr->uf == 1)
		{
			tr->set_y(cl->get_min_y() - 5);
			nor->type = Normal::Air;
			if (nor->get_owner_component<Avatar>() != nullptr)
			{
				nor->get_owner_component<Avatar>()->switch_act(Avatar::ACTION::JUMP);
			}
			nor->vspeed = 0;
		}
		else
		{
			// 绳子顶端封住
			if (nor->get_owner_component<Avatar>() != nullptr)
			{
				auto ava = nor->get_owner_component<Avatar>()->animate = false;
			}
		}
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
	// 快速排斥实验
	if ((p1.x > p2.x ? p1.x : p2.x) < (p3.x < p4.x ? p3.x : p4.x) ||
		(p1.y > p2.y ? p1.y : p2.y) < (p3.y < p4.y ? p3.y : p4.y) ||
		(p3.x > p4.x ? p3.x : p4.x) < (p1.x < p2.x ? p1.x : p2.x) ||
		(p3.y > p4.y ? p3.y : p4.y) < (p1.y < p2.y ? p1.y : p2.y))
	{
		return std::nullopt;
	}
	// 跨立实验
	if ((((p1.x - p3.x) * (p4.y - p3.y) - (p1.y - p3.y) * (p4.x - p3.x)) *
		 ((p2.x - p3.x) * (p4.y - p3.y) - (p2.y - p3.y) * (p4.x - p3.x))) > 0 ||
		(((p3.x - p1.x) * (p2.y - p1.y) - (p3.y - p1.y) * (p2.x - p1.x)) *
		 ((p4.x - p1.x) * (p2.y - p1.y) - (p4.y - p1.y) * (p2.x - p1.x))) > 0)
	{
		return std::nullopt;
	}

	auto x = ((p1.y - p3.y) * (p2.x - p1.x) * (p4.x - p3.x) + p3.x * (p4.y - p3.y) * (p2.x - p1.x) - p1.x * (p2.y - p1.y) * (p4.x - p3.x)) / ((p4.x - p3.x) * (p1.y - p2.y) - (p2.x - p1.x) * (p3.y - p4.y));
	auto y = (p2.y * (p1.x - p2.x) * (p4.y - p3.y) + (p4.x - p2.x) * (p4.y - p3.y) * (p1.y - p2.y) - p4.y * (p3.x - p4.x) * (p2.y - p1.y)) / ((p1.x - p2.x) * (p4.y - p3.y) - (p2.y - p1.y) * (p3.x - p4.x));

	return SDL_FPoint{x, y};
}
