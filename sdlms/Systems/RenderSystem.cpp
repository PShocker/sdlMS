#include "RenderSystem.h"

#include "Core/World.h"
#include "Core/ECSSystem.h"

#include "Entities/Tile.h"

void RenderSystem::run(World &world)
{
	if (world.components_exist_of_type<Transform>())
	{
		for (auto *tr : world.get_components<Transform>())
		{
			render_sprite(tr, tr->get_owner_component<Sprite>(), world);
		}
	}
}

void RenderSystem::render_sprite(Transform *tr, Sprite *spr, World &world)
{
	if (tr)
	{
		float rot = tr->get_rotation();

		auto width = spr->get_width();
		auto heihgt = spr->get_height();

		auto x = tr->get_position().x;
		auto y = tr->get_position().y;

		const SDL_FPoint origin{(float)spr->get_origin().x, (float)spr->get_origin().y};
		const SDL_FRect pos_rect{(float)x - origin.x, (float)y - origin.y, (float)width, (float)heihgt};

		SDL_RenderCopyExF(Window::get_renderer(), spr->get_texture(), nullptr, &pos_rect, rot, &origin, (SDL_RendererFlip)tr->get_flip());
	}
}
