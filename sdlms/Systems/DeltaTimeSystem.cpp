#include "DeltaTimeSystem.h"
#include <ranges>
#include <algorithm>
#include <SDL2/SDL.h>

void DeltaTimeSystem::run(World &world)
{
	auto delta_time = world.get_delta_time();

	if (world.components_exist_of_type<AnimatedSprite>())
	{
		for (auto &[index, aspr] : world.get_components<AnimatedSprite>())
		{
			if (aspr->get_owner() != nullptr)
			{
				update_animated_sprite(aspr, delta_time, world);
			}
		}
	}
	if (world.components_exist_of_type<Avatar>())
	{
		for (auto &[index, ava] : world.get_components<Avatar>())
		{
			update_avatar(ava, delta_time, world);
		}
	}
	if (world.entity_exist_of_type<Timer>())
	{
		update_timer(delta_time, world);
	}
}

bool DeltaTimeSystem::update_animated_sprite(AnimatedSprite *aspr, int delta_time, World &world)
{
	bool end = false;
	[[likely]]
	if (aspr->get_animate())
	{
		aspr->add_anim_time(delta_time);
		if (aspr->get_anim_time() >= aspr->get_anim_delay())
		{
			if (aspr->get_anim_index() == aspr->get_anim_size() - 1 && aspr->get_zigzag())
			{
				std::ranges::reverse(aspr->get_sprites());
				aspr->set_anim_index(1);
			}
			else
			{
				aspr->set_anim_index((aspr->get_anim_index() + 1) % aspr->get_anim_size());
			}
			aspr->set_anim_time(0);
			end = true;
		}
		// 透明度处理
		auto a0 = aspr->get_current_sprite()->a0;
		auto a1 = aspr->get_current_sprite()->a1;
		[[unlikely]]
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
		else
		{
			SDL_SetTextureAlphaMod(aspr->get_current_sprite()->texture, a0);
		}
	}
	return end;
}

void DeltaTimeSystem::update_avatar(Avatar *ava, int delta_time, World &world)
{
	[[likely]]
	if (ava->animate)
	{
		auto delay = ava->stance_delays[ava->act][ava->act_index];
		ava->act_time += delta_time;
		if (ava->act_time >= delay)
		{
			ava->act_index = (ava->act_index + 1) % ava->stance_delays[ava->act].size();
			ava->act_time = 0;
		}
	}
}

void DeltaTimeSystem::update_timer(int delta_time, World &world)
{
	auto &target_map = world.get_entitys<Timer>();
	for (auto it = target_map.begin(); it != target_map.end();)
	{
		it->second->time -= delta_time;
		if (it->second->time <= 0)
		{
			auto time = it->second->callback(0, it->second);
			if (time > 0)
			{
				it->second->time = time;
			}
			else
			{
				it = target_map.erase(it);
				continue;
			}
		}
		++it;
	}
}
