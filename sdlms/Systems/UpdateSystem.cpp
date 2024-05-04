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
}

bool UpdateSystem::update_animated_sprite(AnimatedSprite *aspr, World &world)
{
	auto delta_time = world.delta_time();
	aspr->add_anim_time(delta_time);
	if (aspr->get_anim_time() >= aspr->get_anim_delay())
	{
		aspr->advance_anim();
		aspr->set_anim_time(0);
	}
	return true;
}

void UpdateSystem::update_avatar(Avatar *ava, Transform *tr, World &world)
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
