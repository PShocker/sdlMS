#include "UpdateSystem.h"

#include <SDL2/SDL.h>

void UpdateSystem::run(World &world)
{
	if (world.components_exist_of_type<AnimatedSprite>())
	{
		for (auto &[index, aspr] : world.get_components<AnimatedSprite>())
		{
			update_animated_sprite(aspr, world);
		}
	}
	if (world.components_exist_of_type<Avatar>())
	{
		for (auto &[index, ava] : world.get_components<Avatar>())
		{
			update_avatar(ava, ava->get_owner_component<Transform>(), world);
		}
	}
	// 更新三段式传送门状态
	if (world.entity_exist_of_type<Portal>())
	{
		for (auto &[index, por] : world.get_entitys<Portal>())
		{
			if (por->aspr_map.size() == 3)
			{
				if (world.components_exist_of_type<Player>())
				{
					auto player = world.get_components<Player>().begin()->second;
					auto tr = player->get_owner_component<Transform>();
					update_portal(por, tr, world);
				}
			}
		}
	}
	if (world.components_exist_of_type<Task>())
	{
		auto &tasks = world.get_components<Task>();
		for (auto it = tasks.begin(); it != tasks.end();)
		{
			auto tas = it->second;
			if (update_task(tas, world))
			{
				it = tasks.erase(it); // 删除匹配值的元素，并返回指向下一个元素的迭代器
				delete tas;
			}
			else
			{
				++it;
			}
		}
	}
}

bool UpdateSystem::update_animated_sprite(AnimatedSprite *aspr, World &world)
{
	bool end = false;
	auto delta_time = world.delta_time();
	aspr->add_anim_time(delta_time);
	if (aspr->get_anim_time() >= aspr->get_anim_delay())
	{
		aspr->advance_anim();
		aspr->set_anim_time(0);
		end = true;
	}
	// 透明度处理
	auto a0 = aspr->get_current_sprite()->a0;
	auto a1 = aspr->get_current_sprite()->a1;
	if (a0 != a1)
	{
		auto alpha = 255;
		if (a0 <= a1)
		{
			alpha = (float)a0 + (float)(a1 - a0) / (float)aspr->get_current_sprite()->delay * (float)aspr->get_anim_time();
		}
		else
		{
			alpha = (float)a0 - (float)(a0 - a1) / (float)aspr->get_current_sprite()->delay * (float)aspr->get_anim_time();
		}
		SDL_SetTextureAlphaMod(aspr->get_current_sprite()->texture, alpha);
	}
	return end;
}

void UpdateSystem::update_avatar(Avatar *ava, Transform *tr, World &world)
{
	if (ava->animate)
	{
		auto delta_time = world.delta_time();
		auto delay = ava->stance_delays[ava->act][ava->act_index];
		ava->act_time += delta_time;
		if (ava->act_time >= delay)
		{
			ava->act_index = (ava->act_index + 1) % ava->stance_delays[ava->act].size();
			ava->act_time = 0;
		}
	}
}

void UpdateSystem::update_portal(Portal *por, Transform *tr, World &world)
{
	auto tran = por->get_component<Transform>(); // 传送门坐标
	auto aspr = por->get_component<AnimatedSprite>();
	auto dis = distance(tran->get_position(), tr->get_position());

	if (dis < 400 && aspr != por->aspr_map[u"portalContinue"])
	{
		aspr = por->aspr_map[u"portalContinue"];
		por->add_component(aspr);
	}
	if (400 <= dis && dis < 600 && aspr != por->aspr_map[u"portalExit"])
	{
		aspr = por->aspr_map[u"portalExit"];
		por->add_component(aspr);
	}
	if (dis >= 600)
	{
		// 距离太远,直接隐藏
		if (aspr != nullptr)
		{
			por->remove_component<AnimatedSprite>();
		}
	}
}

bool UpdateSystem::update_task(Task *tas, World &world)
{
	auto delta_time = world.delta_time();
	tas->delay -= delta_time;
	if (tas->delay <= 0)
	{
		tas->func(tas->get_owner(), world);
		return true;
	}
	return false;
}
