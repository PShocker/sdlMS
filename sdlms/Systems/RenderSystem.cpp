#include "RenderSystem.h"

#include "Core/World.h"
#include "Core/ECSSystem.h"

#include "Entities/Tile.h"

void RenderSystem::run(World &world)
{
	if (world.components_exist_of_type<Tile>())
	{
		for (auto *spr : world.get_components<Sprite>())
		{
			render_sprite(spr, spr->get_owner_component<Transform>(), world);
		}
	}
}

void RenderSystem::render_sprite(Sprite *spr, Transform *tr, World &world)
{
	if (tr)
	{
		float rot = tr->get_rotation();

		auto width = spr->get_width();
		auto heihgt = spr->get_height();

		auto x = tr->get_position().x;
		auto y = tr->get_position().y;

		const SDL_FRect pos_rect{(float)x, (float)y, (float)width, (float)heihgt};
		const SDL_FPoint origin{(float)spr->get_origin().x, (float)spr->get_origin().y};

		SDL_RenderCopyExF(Window::get_renderer(), spr->get_texture(), nullptr, &pos_rect, rot, &origin, (SDL_RendererFlip)tr->get_flip());
	}
}
