#include "RenderSystem.h"

#include "Core/World.h"
#include "Core/ECSSystem.h"

void RenderSystem::run(World &world)
{
	if (world.components_exist_of_type<Transform>())
	{
		for (auto &[index, tr] : world.get_components<Transform>())
		{
			if (tr->get_owner_component<HVTile>() != nullptr)
			{
				render_hvtile_sprite(tr, tr->get_owner_component<HVTile>(), world);
			}
			else if (tr->get_owner_component<Sprite>() != nullptr)
			{
				render_sprite(tr, tr->get_owner_component<Sprite>(), world);
			}
			else
			{
				render_animated_sprite(tr, tr->get_owner_component<AnimatedSprite>(), world);
			}
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

void RenderSystem::render_animated_sprite(Transform *tr, AnimatedSprite *aspr, World &world)
{
	if (tr)
	{
		auto spr = aspr->sprites[aspr->anim_index];
		render_sprite(tr, spr, world);
	}
}

void RenderSystem::render_hvtile_sprite(Transform *tr, HVTile *hvt, World &world)
{
	auto spr_w = 0;
	auto spr_h = 0;
	if (tr->get_owner_component<Sprite>() != nullptr)
	{
		spr_w = tr->get_owner_component<Sprite>()->get_width();
		spr_h = tr->get_owner_component<Sprite>()->get_height();
	}
	else
	{
		spr_w = tr->get_owner_component<AnimatedSprite>()->get_anim_width();
		spr_h = tr->get_owner_component<AnimatedSprite>()->get_anim_height();
	}

	SDL_FPoint point = tr->get_position();
	SDL_FRect viewprot = {0, 0, 1200, 768};

	point.x += viewprot.x + viewprot.w / 2;
	point.y += viewprot.y + viewprot.h / 2;

	int cx = 0;
	int cy = 0;

	auto tile_cnt_x = 1;
	if (hvt->get_cx().has_value())
	{
		cx = hvt->get_cx().value();
		if (cx == 0)
		{
			cx = spr_w;
		}
		auto tile_start_right = int(point.x + spr_w - viewprot.x) % cx;
		if (tile_start_right <= 0)
		{
			tile_start_right = tile_start_right + cx;
		}
		tile_start_right = tile_start_right + viewprot.x;

		auto tile_start_left = tile_start_right - spr_w;
		if (tile_start_left >= viewprot.x + viewprot.w)
		{
			tile_cnt_x = 0;
		}
		else
		{
			tile_cnt_x = ceil((viewprot.x + viewprot.w - tile_start_left) / float(cx));
			point.x = tile_start_left;
		}
	}

	auto tile_cnt_y = 1;
	if (hvt->get_cy().has_value())
	{
		cy = hvt->get_cy().value();
		if (cy == 0)
		{
			cy = spr_h;
		}
		auto tile_start_bottom = int(point.y + spr_h - viewprot.y) % cy;
		if (tile_start_bottom <= 0)
		{
			tile_start_bottom = tile_start_bottom + cy;
		}
		tile_start_bottom = tile_start_bottom + viewprot.y;

		auto tile_start_top = tile_start_bottom - spr_h;
		if (tile_start_top >= viewprot.y + viewprot.h)
		{
			tile_cnt_y = 0;
		}
		else
		{
			tile_cnt_y = ceil((viewprot.y + viewprot.h - tile_start_top) / float(cy));
			point.y = tile_start_top;
		}
	}

	for (int i = 0; i < tile_cnt_y; i++)
	{
		for (int j = 0; j < tile_cnt_x; j++)
		{
			Transform *t = new Transform{(float)point.x + j * cx, (float)point.y + i * cy};
			if (tr->get_owner_component<Sprite>() != nullptr)
			{
				render_sprite(t, tr->get_owner_component<Sprite>(), world);
			}
			else
			{
				render_animated_sprite(t, tr->get_owner_component<AnimatedSprite>(), world);
			}
			delete t;
		}
	}
}
