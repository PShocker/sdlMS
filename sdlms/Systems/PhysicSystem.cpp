#include <utility>
#include <algorithm>

#include "PhysicSystem.h"
#include "Entities/FootHold.h"
#include "Entities/LadderRope.h"
#include "Entities/Character.h"
#include "Entities/Mob.h"
#include "Entities/Npc.h"
#include "Entities/Portal.h"
#include "Entities/Timer.h"
#include "Components/Line.h"
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
			break;
		}
		// 判断是否爬梯子
		[[unlikely]]
		if (want_climb(tr, nor, world))
		{
			break;
		}
		// 下跳
		[[unlikely]]
		if (want_fall(tr, nor, world))
		{
			break;
		}
		// 趴
		[[unlikely]]
		if (want_prone(nor, world))
		{
			break;
		}
		// 跳
		[[unlikely]]
		if (want_jump(tr, nor, world))
		{
			break;
		}
		// 攻击
		[[unlikely]]
		if (want_attack(tr, nor, world))
		{
			break;
		}
		// 地面移动判断
		[[unlikely]]
		if (!walk(tr, nor, world, delta_time))
		{
			break;
		}
		[[likely]]
		if (want_stand(nor, world))
		{
			break;
		}
		break;
	case Normal::Air:
		// 判断是否进传送门
		[[unlikely]]
		if (want_portal(tr, nor, world))
		{
			break;
		}
		// 判断是否爬梯子
		[[unlikely]]
		if (want_climb(tr, nor, world))
		{
			break;
		}
		// 空中
		fall(tr, nor, delta_time, world);
		break;
	case Normal::Climb:
		// 是否进传送门
		[[unlikely]]
		if (want_portal(tr, nor, world))
		{
			break;
		}
		// 爬梯子
		[[unlikely]]
		if (want_jump(tr, nor, world))
		{
			break;
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
			auto line = lr->get_component<Line>();

			// 判断x坐标是否在梯子范围内
			if (pos.x >= line->get_min_x() - 10 && pos.x <= line->get_max_x() + 10)
			{
				if (nor->vkey == Normal::Up)
				{
					if (nor->type == Normal::Ground)
					{
						// 地面向上爬
						if (pos.y > line->get_max_y() && pos.y <= line->get_max_y() + 5)
						{
							// 爬到梯子
							lad = lr;
							break;
						}
					}
					else if (nor->type == Normal::Air)
					{
						// 空中向上爬
						if (pos.y > line->get_min_y() && pos.y < line->get_max_y() + 5)
						{
							// 爬到梯子
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
						if (pos.y < line->get_min_y() && pos.y >= line->get_min_y() - 5)
						{
							lad = lr;
							break;
						}
					}
				}
			}
		}
		if (lad != nullptr)
		{
			// 爬到了梯子上
			auto foo = tr->get_owner()->get_entity<FootHold>(0);
			foo->zmass = 0;

			auto line = lad->get_component<Line>();

			tr->set_x(line->get_m().x);
			tr->set_y(std::clamp(tr->get_position().y, line->get_min_y(), line->get_max_y()));

			if (nor->get_owner<Character>() != nullptr)
			{
				auto cha = nor->get_owner<Character>();
				// 判断爬的是梯子还是绳子
				if (lad->l == 1)
				{
					// 梯子
					cha->switch_act(Avatar::ACTION::LADDER);
				}
				else
				{
					// 绳子
					cha->switch_act(Avatar::ACTION::ROPE);
				}
				cha->stop_animate();
			}
			// 修改人物z值
			world.destroy_component(tr, false);
			world.add_component(tr, lad->page * 300000 + 40000);

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
		if (nor->get_owner<Character>() != nullptr)
		{
			nor->get_owner<Character>()->switch_act(Avatar::ACTION::PRONE);
		}
		else if (nor->get_owner<Mob>() != nullptr)
		{
			nor->get_owner<Mob>()->switch_act(u"stand");
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
		for (auto &[id, fh] : world.get_entitys<FootHold>())
		{
			auto line = fh->get_component<Line>();
			if (line->get_y(tr->get_position().x).has_value() &&
				line->get_y(tr->get_position().x).value() < tr->get_position().y + 600 &&
				line->get_y(tr->get_position().x).value() > tr->get_position().y &&
				fh != tr->get_owner()->get_entity<FootHold>(0))
			{
				// 找到了可下跳的fh
				auto foo = tr->get_owner()->get_entity<FootHold>(0);
				world.remove_entity(foo);
				world.add_entity(foo, -foo->get_id());

				// 添加定时任务到world
				auto callback = [](Uint32 interval, void *param) -> Uint32
				{
					auto world = World::get_world();
					auto timer = (Timer *)param;
					auto foo = timer->get_entity<FootHold>(0);
					world->remove_entity(foo);
					world->add_entity(foo, -foo->get_id());
					foo->remove_entity(timer);
					delete timer;
					return 0;
				};
				Timer *timer = new Timer(callback, 400);
				world.add_entity(timer);
				foo->add_entity(timer);
				timer->add_entity(foo, 0);

				// 从fh掉落
				nor->type = Normal::Air;
				if (nor->get_owner<Character>() != nullptr)
				{
					nor->get_owner<Character>()->switch_act(Avatar::ACTION::JUMP);
				}
				else if (nor->get_owner<Mob>() != nullptr)
				{
					nor->get_owner<Mob>()->switch_act(u"jump");
				}
				nor->vspeed = -150;
				nor->hspeed = 0;
				// 修改人物z值
				world.destroy_component(tr, false);
				world.add_component(tr, 7 * 300000 + 40000);
				return true;
			}
		}
	}
	return false;
}

bool PhysicSystem::want_stand(Normal *nor, World &world)
{
	if (nor->hkey == Normal::None)
	{
		// 还需要进行alert状态判断
		if (nor->get_owner<Character>() != nullptr)
		{
			nor->get_owner<Character>()->switch_act(Avatar::ACTION::STAND1);
		}
		else if (nor->get_owner<Mob>() != nullptr)
		{
			nor->get_owner<Mob>()->switch_act(u"stand");
		}
		else if (nor->get_owner<Npc>() != nullptr)
		{
			nor->get_owner<Npc>()->switch_act(u"stand");
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

			if (nor->get_owner<Character>() != nullptr)
			{
				nor->get_owner<Character>()->switch_act(Avatar::ACTION::JUMP);
			}
			else if (nor->get_owner<Mob>() != nullptr)
			{
				nor->get_owner<Mob>()->switch_act(u"jump");
			}
			return true;
		}
		else if (nor->type == Normal::Climb && (nor->hkey == Normal::Right || nor->hkey == Normal::Left) && (nor->vkey != Normal::Up))
		{
			nor->vspeed = -310;
			nor->type = Normal::Air;
			if (nor->hkey == Normal::Right)
			{
				nor->hspeed = 140;
			}
			else if (nor->hkey == Normal::Left)
			{
				nor->hspeed = -140;
			}

			if (nor->get_owner<Character>() != nullptr)
			{
				// 修改纸娃娃状态
				nor->get_owner<Character>()->switch_act(Avatar::ACTION::JUMP);
				nor->get_owner<Character>()->start_animate();
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

bool PhysicSystem::want_attack(Transform *tr, Normal *nor, World &world)
{
	if (nor->lctrl)
	{
		if (nor->get_owner<Character>() != nullptr)
		{
			// 生成0到5之间的随机数
			// int randomNum = std::rand() % 6;
			int randomNum = 0;
			auto cha = nor->get_owner<Character>();
			// ava->switch_act(Avatar::ACTION::STABO1);
			switch (randomNum)
			{
			case 0:
				cha->switch_act(Avatar::ACTION::STABO1);
				break;
			case 1:
				cha->switch_act(Avatar::ACTION::STABO2);
				break;
			case 2:
				cha->switch_act(Avatar::ACTION::STABOF);
				break;
			case 3:
				cha->switch_act(Avatar::ACTION::STABT1);
				break;
			case 4:
				cha->switch_act(Avatar::ACTION::STABT2);
				break;
			case 5:
				cha->switch_act(Avatar::ACTION::STABTF);
				break;
			default:
				cha->switch_act(Avatar::ACTION::STABO1);
				break;
			}
		}
		else if (nor->get_owner<Mob>() != nullptr)
		{
			nor->get_owner<Mob>()->switch_act(u"attack");
		}
		return true;
	}
	return false;
}

bool PhysicSystem::want_portal(Transform *tr, Normal *nor, World &world)
{
	if (tr->get_owner_component<Player>() != nullptr)
	{
		Portal *por = nullptr;
		// 先判断接触式传送门
		for (auto &[id, p] : world.get_entitys<Portal>())
		{
			if (p->tm == 999999999 || id < 0)
			{
				// 屏蔽脚本传送门和冷却中的传送门
				continue;
			}
			if (p->pt == 3 || nor->vkey == Normal::Up)
			{
				auto pla_pos = tr->get_position();
				auto por_pos = p->get_component<Transform>();
				auto por_x = por_pos->get_position().x;
				auto por_y = por_pos->get_position().y;
				if (pla_pos.x == std::clamp(pla_pos.x, por_x - 20, por_x + 20) &&
					pla_pos.y == std::clamp(pla_pos.y, por_y - 50, por_y + 50))
				{
					por = p;
					break;
				}
			}
		}
		if (por != nullptr)
		{
			// 切换地图
			auto tn = por->tn;
			if (por->tm != Map::get_map_id())
			{
				// 切换地图
				Map::load(por->tm, &world);
				world.tick_delta_time();
				// 切换人物坐标
				for (auto &[id, p] : world.get_entitys<Portal>())
				{
					if (tn == p->pn)
					{
						por = p;
						tr->set_position(por->get_component<Transform>()->get_position() + SDL_FPoint{0, -10});
						// 调整相机位置
						auto camera = world.get_components<Camera>().find(0)->second;
						camera->set_x(tr->get_position().x - camera->get_w() / 2);
						camera->set_y(tr->get_position().y - camera->get_h() / 2);
						break;
					}
				}
			}
			else
			{
				// 切换人物坐标
				for (auto &[id, p] : world.get_entitys<Portal>())
				{
					if (tn == p->pn)
					{
						por = p;
						tr->set_position(por->get_component<Transform>()->get_position() + SDL_FPoint{0, -5});
						break;
					}
				}
			}
			// 设置传送门冷却
			world.remove_entity(por);
			world.add_entity(por, -por->get_id());
			// 添加定时任务到world
			auto callback = [](Uint32 interval, void *param) -> Uint32
			{
				auto world = World::get_world();
				auto timer = (Timer *)param;
				auto por = timer->get_entity<Portal>(0);
				world->remove_entity(por);
				world->add_entity(por, -por->get_id());
				por->remove_entity(timer);
				delete timer;
				return 0;
			};
			Timer *timer = new Timer(callback, 800);
			world.add_entity(timer);

			por->add_entity(timer);
			timer->add_entity(por, 0);

			// 修改人物z值
			world.destroy_component(tr, false);
			world.add_component(tr, 7 * 300000 + 40000);

			nor->type = Normal::Air;
			if (nor->get_owner<Character>() != nullptr)
			{
				// 修改纸娃娃状态
				nor->get_owner<Character>()->switch_act(Avatar::ACTION::JUMP);
			}
			else if (nor->get_owner<Mob>() != nullptr)
			{
				nor->get_owner<Mob>()->switch_act(u"jump");
			}
			nor->vspeed = 0;
			nor->hspeed = 0;
			return true;
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
		if (nor->get_owner<Character>() != nullptr)
		{
			nor->get_owner<Character>()->switch_act(Avatar::ACTION::WALK1);
		}
		else if (nor->get_owner<Mob>() != nullptr)
		{
			nor->get_owner<Mob>()->switch_act(u"move");
		}
		else if (nor->get_owner<Npc>() != nullptr)
		{
			nor->get_owner<Npc>()->switch_act(u"move");
		}
	}
	else
	{
		nor->hforce = 0;
		// 如果没有左右的输入并且速度为0,则可以直接return提高性能
		if (nor->hspeed == 0)
		{
			return true;
		}
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
	if (nor->hspeed_limit.has_value())
	{
		auto min = nor->hspeed_limit.value().x;
		auto max = nor->hspeed_limit.value().y;
		nor->hspeed = std::clamp(nor->hspeed, min, max);
	}

	auto d_x = nor->hspeed * delta_time;
	auto x = d_x + tr->get_position().x;
	auto y = tr->get_position().y;

	auto &fhs = world.get_entitys<FootHold>();
	auto foo = tr->get_owner()->get_entity<FootHold>(0);
	auto line = foo->get_component<Line>();

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
			// 悬崖边掉落
			nor->type = Normal::Air;
			if (nor->get_owner<Character>() != nullptr)
			{
				nor->get_owner<Character>()->switch_act(Avatar::ACTION::JUMP);
			}
			else if (nor->get_owner<Mob>() != nullptr)
			{
				nor->get_owner<Mob>()->switch_act(u"jump");
			}
			nor->vspeed = 0;
			tr->set_y(y);
			tr->set_x(x);
			return false;
		}
		auto line = fh->get_component<Line>();
		if (!line->get_k().has_value())
		{
			if (y == std::clamp(y, line->get_max_y() - 1, line->get_max_y() + 1))
			{
				// 撞墙
				if (nor->hspeed < 0)
				{
					tr->set_x(line->get_m().x + 0.1);
				}
				else
				{
					tr->set_x(line->get_m().x - 0.1);
				}
				tr->set_y(foo->get_component<Line>()->get_y(tr->get_position().x).value());
				nor->hspeed = 0;
				return false;
			}
			else
			{
				// 楼梯上掉落
				nor->type = Normal::Air;
				if (nor->get_owner<Character>() != nullptr)
				{
					nor->get_owner<Character>()->switch_act(Avatar::ACTION::JUMP);
				}
				else if (nor->get_owner<Mob>() != nullptr)
				{
					nor->get_owner<Mob>()->switch_act(u"jump");
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
	while (x < line->get_min_x())
	{
		int next_fh = std::abs(foo->prev);
		if (walk_fh(next_fh) == false)
		{
			// 从fh掉落,撞墙
			return false;
		}
		line = foo->get_component<Line>();
	}
	// 往右走
	while (x > line->get_max_x())
	{
		int next_fh = std::abs(foo->next);
		if (walk_fh(next_fh) == false)
		{
			// 从fh掉落,撞墙
			return false;
		}
		line = foo->get_component<Line>();
	}
	// 地面上
	nor->get_owner()->add_entity(foo, 0);
	tr->set_y(line->get_y(x).value());
	tr->set_x(x);
	return true;
}

void PhysicSystem::fall(Transform *tr, Normal *nor, float delta_time, World &world)
{
	if (nor->hkey == Normal::Right)
	{
		nor->hspeed += 0.3f;
	}
	else if (nor->hkey == Normal::Left)
	{
		nor->hspeed -= 0.3f;
	}
	// 默认重力为2000
	nor->vspeed += delta_time * 2000;
	if (nor->vspeed_limit.has_value())
	{
		auto min = nor->vspeed_limit.value().x;
		auto max = nor->vspeed_limit.value().y;
		nor->vspeed = std::clamp(nor->vspeed, min, max);
	}

	auto d_x = nor->hspeed * delta_time;
	auto d_y = nor->vspeed * delta_time;

	auto new_pos = tr->get_position() + SDL_FPoint{(float)d_x, (float)d_y};

	// 人物之前的fh
	auto foo = tr->get_owner()->get_entity<FootHold>(0);

	auto &fhs = world.get_entitys<FootHold>();

	auto collide_wall = [&fhs](Line *line, FootHold *fh, float hspeed) -> bool
	{
		if (hspeed > 0 && line->get_m().y > line->get_n().y)
		{
			while (fhs.contains(fh->prev))
			{
				fh = fhs.find(fh->prev)->second;
				auto l = fh->get_component<Line>();
				if (l->get_k().has_value())
				{
					return true;
				}
			}
		}
		else if (hspeed < 0 && line->get_m().y < line->get_n().y)
		{
			while (fhs.contains(fh->next))
			{
				fh = fhs.find(fh->next)->second;
				auto l = fh->get_component<Line>();
				if (l->get_k().has_value())
				{
					return true;
				}
			}
		}
		return false;
	};
	// 下落
	if (nor->vspeed > 0)
	{
		for (auto &[id, fh] : fhs)
		{
			if (id < 0 && nor->get_owner_component<Player>() != nullptr)
			{
				// id<0表示下跳时的fh,需要排除玩家本身
				continue;
			}
			auto line = fh->get_component<Line>();
			auto collide = intersect(tr->get_position(), new_pos, line->get_m(), line->get_n());
			if (collide.has_value())
			{
				if (!line->get_k().has_value())
				{
					// 判断墙面碰撞方向
					if (collide_wall(line, fh, nor->hspeed) == true)
					{
						if (nor->hspeed < 0)
						{
							new_pos.x = line->get_m().x + 0.1;
						}
						else
						{
							new_pos.x = line->get_m().x - 0.1;
						}
						nor->hspeed = 0;
					}
				}
				else
				{
					// 落地
					new_pos.x = std::clamp(collide.value().x, line->get_min_x(), line->get_max_x());
					new_pos.y = line->get_y(new_pos.x).value();
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
					if (nor->get_owner<Character>() != nullptr)
					{
						world.add_component(tr, fh->page * 300000 + 40000);
					}
					else if (nor->get_owner<Mob>() != nullptr)
					{
						auto load_id = tr->get_id() % 1000;
						world.add_component(tr, fh->page * 300000 + load_id + 30000);
					}
					else if (nor->get_owner<Npc>() != nullptr)
					{
						auto load_id = tr->get_id() % 1000;
						world.add_component(tr, fh->page * 300000 + load_id + 20000);
					}
					break;
				}
			}
		}
	}
	else if (nor->vspeed <= 0)
	{
		// 上升
		// 跳跃时只用考虑是否撞墙
		for (auto &[id, fh] : fhs)
		{
			auto line = fh->get_component<Line>();
			if (line->get_n().y == line->get_m().y &&
				line->get_n().x < line->get_m().x &&
				(fh->zmass == 0 ||
				 (foo != nullptr &&
				  fh->zmass == foo->zmass)))
			{
				// 天花板
				auto collide = intersect(tr->get_position(), new_pos, line->get_m(), line->get_n());
				if (collide.has_value())
				{
					new_pos = collide.value();
					new_pos.y += 0.1;
					nor->hspeed = 0;
					nor->vspeed = 0;
					break;
				}
			}
			if (!line->get_k().has_value())
			{
				auto collide = intersect(tr->get_position(), new_pos, line->get_m(), line->get_n());
				if (collide.has_value())
				{
					if (collide_wall(line, fh, nor->hspeed) == true)
					{
						if (nor->hspeed < 0)
						{
							new_pos.x = line->get_m().x + 0.1;
						}
						else
						{
							new_pos.x = line->get_m().x - 0.1;
						}
						nor->hspeed = 0;
						break;
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
	auto line = lr->get_component<Line>();

	auto d_y = nor->vspeed * delta_time;
	auto y = d_y + tr->get_position().y;
	if (y < line->get_min_y())
	{
		if (lr->uf == 1)
		{
			tr->set_y(line->get_min_y() - 5);
			nor->type = Normal::Air;
			if (nor->get_owner<Character>() != nullptr)
			{
				nor->get_owner<Character>()->switch_act(Avatar::ACTION::JUMP);
				nor->get_owner<Character>()->start_animate();
			}
			nor->vspeed = 0;
		}
		else
		{
			// 绳子顶端封住
			if (nor->get_owner<Character>() != nullptr)
			{
				nor->get_owner<Character>()->stop_animate();
			}
		}
	}
	else if (y > line->get_max_y())
	{
		tr->set_y(line->get_max_y());
		nor->type = Normal::Air;
		if (nor->get_owner<Character>() != nullptr)
		{
			nor->get_owner<Character>()->switch_act(Avatar::ACTION::JUMP);
			nor->get_owner<Character>()->start_animate();
		}
		nor->vspeed = 0;
	}
	else
	{
		if (nor->get_owner<Character>() != nullptr)
		{
			if (nor->vspeed != 0)
			{
				nor->get_owner<Character>()->start_animate();
			}
			else
			{
				nor->get_owner<Character>()->stop_animate();
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
