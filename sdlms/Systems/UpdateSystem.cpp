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
}

bool UpdateSystem::update_animated_sprite(AnimatedSprite *aspr, World &world)
{
	auto delta_time = world.delta_time();
	// printf("%d\n", delta_time);
	aspr->add_anim_time(delta_time);
	if (aspr->get_anim_time() >= aspr->get_anim_delay())
	{
		aspr->advance_anim();
		aspr->set_anim_time(0);
	}
	return true;
}
