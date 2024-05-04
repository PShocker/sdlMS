#include "RenderSystem.h"

#include "Core/World.h"
#include "Core/ECSSystem.h"
#include "Components/Camera.h"

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
			else if (tr->get_owner_component<Avatar>() != nullptr)
			{
				render_avatar_sprite(tr, tr->get_owner_component<Avatar>(), world);
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
		if (tr->get_camera())
		{
			// 跟随摄像机的sprite
			const SDL_FRect pos_rect{(float)x - origin.x, (float)y - origin.y, (float)width, (float)heihgt};
			SDL_RenderCopyExF(Window::get_renderer(), spr->get_texture(), nullptr, &pos_rect, rot, &origin, (SDL_RendererFlip)tr->get_flip());
		}
		else
		{
			// 不随摄像机移动
			auto camera = world.get_components<Camera>().find(0)->second;
			const SDL_FRect pos_rect{(float)x - origin.x - camera->get_x(), (float)y - origin.y - camera->get_y(), (float)width, (float)heihgt};
			SDL_RenderCopyExF(Window::get_renderer(), spr->get_texture(), nullptr, &pos_rect, rot, &origin, (SDL_RendererFlip)tr->get_flip());
		}
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
	auto viewprot_x = world.get_components<Camera>().find(0)->second->get_x();
	auto viewprot_y = world.get_components<Camera>().find(0)->second->get_y();
	auto viewprot_w = world.get_components<Camera>().find(0)->second->get_w();
	auto viewprot_h = world.get_components<Camera>().find(0)->second->get_h();

	auto spr_w = 0;
	auto spr_h = 0;

	float spr_ox = 0;
	float spr_oy = 0;
	if (tr->get_owner_component<Sprite>() != nullptr)
	{
		spr_w = tr->get_owner_component<Sprite>()->get_width();
		spr_h = tr->get_owner_component<Sprite>()->get_height();
		spr_ox = tr->get_owner_component<Sprite>()->get_origin().x;
		spr_oy = tr->get_owner_component<Sprite>()->get_origin().y;
	}
	else
	{
		spr_w = tr->get_owner_component<AnimatedSprite>()->get_anim_width();
		spr_h = tr->get_owner_component<AnimatedSprite>()->get_anim_height();
		spr_ox = tr->get_owner_component<AnimatedSprite>()->get_current_sprite()->get_origin().x;
		spr_oy = tr->get_owner_component<AnimatedSprite>()->get_current_sprite()->get_origin().y;
	}
	auto hvm = tr->get_owner_component<HVMove>();

	int rx = hvm->get_rx();
	int ry = hvm->get_ry();

	int cx = hvt->get_cx();
	int cy = hvt->get_cy();
	if (cx == 0)
	{
		cx = spr_w;
	}
	if (cy == 0)
	{
		cy = spr_h;
	}
	if (hvm->get_hspeed())
	{
		hvm->offset_x = fmodf(hvm->offset_x + rx * 5 * world.delta_time() / 1000.0, cx);
	}
	else
	{
		hvm->offset_x = (viewprot_x + viewprot_w / 2) * (rx + 100) / 100.0;
	}
	if (hvm->get_vspeed())
	{
		hvm->offset_y = fmodf(hvm->offset_y + ry * 5 * world.delta_time() / 1000.0, cy);
	}
	else
	{
		hvm->offset_y = (viewprot_y + viewprot_h / 2) * (ry + 100) / 100.0;
	}

	SDL_FPoint point = tr->get_position() - SDL_FPoint{spr_ox, spr_oy};

	point.x += hvm->offset_x;
	point.y += hvm->offset_y;

	auto tile_cnt_x = 1;
	if (hvt->htile && cx > 0)
	{
		auto tile_start_right = int(point.x + spr_w - viewprot_x) % cx;
		if (tile_start_right <= 0)
		{
			tile_start_right = tile_start_right + cx;
		}
		tile_start_right = tile_start_right + viewprot_x;

		auto tile_start_left = tile_start_right - spr_w;
		if (tile_start_left >= viewprot_x + viewprot_w)
		{
			tile_cnt_x = 0;
		}
		else
		{
			tile_cnt_x = ceil((viewprot_x + viewprot_w - tile_start_left) / float(cx));
			point.x = tile_start_left;
		}
	}

	auto tile_cnt_y = 1;
	if (hvt->vtile && cy > 0)
	{
		auto tile_start_bottom = int(point.y + spr_h - viewprot_y) % cy;
		if (tile_start_bottom <= 0)
		{
			tile_start_bottom = tile_start_bottom + cy;
		}
		tile_start_bottom = tile_start_bottom + viewprot_y;

		auto tile_start_top = tile_start_bottom - spr_h;
		if (tile_start_top >= viewprot_y + viewprot_h)
		{
			tile_cnt_y = 0;
		}
		else
		{
			tile_cnt_y = ceil((viewprot_y + viewprot_h - tile_start_top) / float(cy));
			point.y = tile_start_top;
		}
	}

	for (int i = 0; i < tile_cnt_y; i++)
	{
		for (int j = 0; j < tile_cnt_x; j++)
		{
			Transform *t = new Transform{(float)point.x + j * cx + spr_ox, (float)point.y + i * cy + spr_oy};
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

void RenderSystem::render_avatar_sprite(Transform *tr, Avatar *ava, World &world)
{
	auto chara_pos = tr->get_position();
	auto act = ava->act;
	auto act_index = ava->act_index;
	auto tran = new Transform();
	auto set_tran = [chara_pos, &tran, ava](Transform *t, Sprite *spr)
	{
		if (ava->direct)
		{
			tran->set_flip(0);
			tran->set_position(chara_pos + t->get_position());
		}
		else
		{
			tran->set_flip(1);
			auto x = -t->get_position().x - spr->get_width() + 2 * spr->get_origin().x;
			auto y = t->get_position().y;
			tran->set_position(chara_pos + SDL_FPoint{x, y});
		}
	};
	{
		auto [t, spr] = ava->body[act][act_index];
		set_tran(t, spr);
		render_sprite(tran, spr, world);
	}
	{
		auto [t, spr] = ava->coat[act][act_index];
		set_tran(t, spr);
		render_sprite(tran, spr, world);
	}
	{
		auto [t, spr] = ava->pants[act][act_index];
		set_tran(t, spr);
		render_sprite(tran, spr, world);
	}
	{
		auto [t, spr] = ava->head[act][act_index];
		set_tran(t, spr);
		render_sprite(tran, spr, world);
	}
	{
		auto [t, spr] = ava->face[act][act_index];
		set_tran(t, spr);
		render_sprite(tran, spr, world);
	}
	{
		auto &v = ava->hairs[act][act_index];
		for (auto &[t, spr] : v)
		{
			set_tran(t, spr);
			render_sprite(tran, spr, world);
		}
	}
	{
		auto [t, spr] = ava->arm[act][act_index];
		set_tran(t, spr);
		render_sprite(tran, spr, world);
	}
	{
		auto [t, spr] = ava->lHand[act][act_index];
		if (t != nullptr && spr != nullptr)
		{
			set_tran(t, spr);
			render_sprite(tran, spr, world);
		}
	}
	delete tran;
}
