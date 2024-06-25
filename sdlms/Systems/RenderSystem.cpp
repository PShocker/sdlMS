#include "RenderSystem.h"

#include "Core/World.h"
#include "Core/ECSSystem.h"
#include "Components/Camera.h"
#include "Components/Avatar.h"

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
			else if (tr->get_owner_component<AnimatedSprite>() != nullptr)
			{
				render_animated_sprite(tr, tr->get_owner_component<AnimatedSprite>(), world);
			}
			else if (tr->get_owner_component<Sprite>() != nullptr)
			{
				render_sprite(tr, tr->get_owner_component<Sprite>(), world);
			}
			else if (tr->get_owner_component<Video>() != nullptr)
			{
				render_video(tr, tr->get_owner_component<Video>(), world);
			}
		}
	}
	// for (auto &[key, val] : world.get_entitys<FootHold>())
	// {
	// 	render_fh(val, world);
	// }
}

void RenderSystem::render_sprite(Transform *tr, Sprite *spr, World &world)
{
	float rot = tr->get_rotation();

	auto width = spr->get_width();
	auto heihgt = spr->get_height();

	auto x = tr->get_position().x;
	auto y = tr->get_position().y;

	const SDL_FPoint origin{(float)spr->get_origin().x, (float)spr->get_origin().y};
	if (tr->get_camera())
	{
		// 显示坐标为绝对坐标,与摄像机无关,通常为ui
		SDL_FRect pos_rect{(float)x - origin.x, (float)y - origin.y, (float)width, (float)heihgt};
		SDL_RenderCopyExF(Window::get_renderer(), spr->get_texture(), nullptr, &pos_rect, rot, &origin, (SDL_RendererFlip)tr->get_flip());
	}
	else
	{
		// 显示坐标与摄像机坐标相关
		auto camera = world.get_components<Camera>().find(0)->second;
		SDL_FRect pos_rect;
		if (tr->get_flip() == 0)
		{
			pos_rect = {(float)x - origin.x - camera->get_x(), (float)y - origin.y - camera->get_y(), (float)width, (float)heihgt};
		}
		else if (tr->get_flip() == 1)
		{
			pos_rect = {(float)x - (spr->get_width() - origin.x) - camera->get_x(), (float)y - origin.y - camera->get_y(), (float)width, (float)heihgt};
		}
		SDL_RenderCopyExF(Window::get_renderer(), spr->get_texture(), nullptr, &pos_rect, rot, &origin, (SDL_RendererFlip)tr->get_flip());
	}
}

void RenderSystem::render_animated_sprite(Transform *tr, AnimatedSprite *aspr, World &world)
{
	auto spr = aspr->get_current_sprite();
	render_sprite(tr, spr, world);
}

void RenderSystem::render_hvtile_sprite(Transform *tr, HVTile *hvt, World &world)
{
	auto delta_time = world.get_delta_time();

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
		hvm->offset_x = fmodf(hvm->offset_x + rx * 5 * delta_time / 1000.0, cx);
	}
	else
	{
		hvm->offset_x = (viewprot_x + viewprot_w / 2) * (rx + 100) / 100.0;
	}
	if (hvm->get_vspeed())
	{
		hvm->offset_y = fmodf(hvm->offset_y + ry * 5 * delta_time / 1000.0, cy);
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
			Transform *t = new Transform((float)point.x + j * cx + spr_ox, (float)point.y + i * cy + spr_oy, tr->get_flip());
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
	auto act = ava->act;
	auto act_index = ava->act_index;
	auto tran = new Transform();
	auto set_tran = [&tr, &tran, ava](Transform *t, Sprite *spr)
	{
		auto chara_pos = tr->get_position();
		auto flip = tr->get_flip();
		if (flip == 0)
		{
			tran->set_flip(0);
			tran->set_position(chara_pos + t->get_position());
		}
		else
		{
			tran->set_flip(1);
			auto x = -t->get_position().x;
			auto y = t->get_position().y;
			tran->set_position(chara_pos + SDL_FPoint{x, y});
		}
	};
	auto render_avatar = [this, &set_tran, &tran, &act, &act_index, &world](std::unordered_map<uint8_t, std::pair<Transform *, Sprite *>> part[Avatar::ACTION::LENGTH])
	{
		if (part[act].size() > 0)
		{
			auto [t, spr] = part[act][act_index];
			if (t != nullptr && spr != nullptr)
			{
				set_tran(t, spr);
				render_sprite(tran, spr, world);
			}
		}
	};
	render_avatar(ava->backTamingMobMid);
	render_avatar(ava->backMobEquipUnderSaddle);
	render_avatar(ava->backSaddle);
	render_avatar(ava->backMobEquipMid);
	render_avatar(ava->backTamingMobFront);
	render_avatar(ava->backMobEquipFront);
	render_avatar(ava->mobEquipRear);
	render_avatar(ava->tamingMobRear);
	render_avatar(ava->saddleRear);
	render_avatar(ava->characterEnd);
	render_avatar(ava->backWeapon);
	if (!ava->cap_vslot.contains(u"Hf"))
	{
		render_avatar(ava->backHairBelowHead);
	}
	render_avatar(ava->backShieldBelowBody);
	render_avatar(ava->backMailChestAccessory);
	render_avatar(ava->backCapAccessory);
	render_avatar(ava->backAccessoryFace);
	render_avatar(ava->backAccessoryEar);
	render_avatar(ava->backBody);
	render_avatar(ava->backGlove);
	render_avatar(ava->backGloveWrist);
	render_avatar(ava->backWeaponOverGlove);
	render_avatar(ava->backMailChestBelowPants);
	render_avatar(ava->backPantsBelowShoes);
	render_avatar(ava->backShoesBelowPants);
	render_avatar(ava->backPants);
	render_avatar(ava->backShoes);
	render_avatar(ava->backPantsOverShoesBelowMailChest);
	render_avatar(ava->backMailChest);
	render_avatar(ava->backPantsOverMailChest);
	render_avatar(ava->backMailChestOverPants);
	render_avatar(ava->backHead);
	render_avatar(ava->backAccessoryFaceOverHead);
	render_avatar(ava->backCape);
	if (!ava->cap_vslot.contains(u"H6"))
	{
		render_avatar(ava->backHairBelowCap);
	}
	if (!ava->cap_vslot.contains(u"H5"))
	{
		render_avatar(ava->backHairBelowCapNarrow);
	}
	if (!ava->cap_vslot.contains(u"H4"))
	{
		render_avatar(ava->backHairBelowCapWide);
	}
	render_avatar(ava->backWeaponOverHead);
	render_avatar(ava->backCap);
	if (!ava->cap_vslot.contains(u"H1"))
	{
		render_avatar(ava->backHair);
	}
	render_avatar(ava->backCapOverHair);
	render_avatar(ava->backShield);
	render_avatar(ava->backWeaponOverShield);
	render_avatar(ava->backWing);
	render_avatar(ava->backHairOverCape);
	render_avatar(ava->weaponBelowBody);
	if (!ava->cap_vslot.contains(u"Hb"))
	{
		render_avatar(ava->hairBelowBody);
	}
	render_avatar(ava->capeBelowBody);
	render_avatar(ava->shieldBelowBody);
	render_avatar(ava->capAccessoryBelowBody);
	render_avatar(ava->gloveBelowBody);
	render_avatar(ava->gloveWristBelowBody);
	render_avatar(ava->body);
	render_avatar(ava->gloveOverBody);
	render_avatar(ava->mailChestBelowPants);
	render_avatar(ava->pantsBelowShoes);
	render_avatar(ava->shoes);
	render_avatar(ava->pants);
	render_avatar(ava->mailChestOverPants);
	render_avatar(ava->shoesOverPants);
	render_avatar(ava->pantsOverShoesBelowMailChest);
	render_avatar(ava->shoesTop);
	render_avatar(ava->mailChest);
	render_avatar(ava->pantsOverMailChest);
	render_avatar(ava->mailChestOverHighest);
	render_avatar(ava->gloveWristOverBody);
	render_avatar(ava->mailChestTop);
	render_avatar(ava->weaponOverBody);
	render_avatar(ava->armBelowHead);
	render_avatar(ava->mailArmBelowHead);
	render_avatar(ava->armBelowHeadOverMailChest);
	render_avatar(ava->gloveBelowHead);
	render_avatar(ava->mailArmBelowHeadOverMailChest);
	render_avatar(ava->gloveWristBelowHead);
	render_avatar(ava->weaponOverArmBelowHead);
	render_avatar(ava->shield);
	render_avatar(ava->weapon);
	render_avatar(ava->arm);
	render_avatar(ava->hand);
	render_avatar(ava->glove);
	render_avatar(ava->mailArm);
	render_avatar(ava->gloveWrist);
	render_avatar(ava->cape);
	render_avatar(ava->head);
	if (!ava->cap_vslot.contains(u"Hs"))
	{
		render_avatar(ava->hairShade);
	}
	render_avatar(ava->accessoryFaceBelowFace);
	if (ava->show_face[ava->act][ava->act_index] == true)
	{
		render_avatar(ava->face);
	}
	render_avatar(ava->accessoryFaceOverFaceBelowCap);
	render_avatar(ava->capBelowAccessory);
	render_avatar(ava->accessoryEar);
	render_avatar(ava->capAccessoryBelowAccFace);
	render_avatar(ava->accessoryFace);
	render_avatar(ava->accessoryEyeShadow);
	render_avatar(ava->accessoryEye);
	if (!ava->cap_vslot.contains(u"H2"))
	{
		render_avatar(ava->hair);
	}
	render_avatar(ava->cap);
	render_avatar(ava->capAccessory);
	render_avatar(ava->accessoryEyeOverCap);
	if (!ava->cap_vslot.contains(u"H1"))
	{
		render_avatar(ava->hairOverHead);
	}
	render_avatar(ava->capOverHair);
	render_avatar(ava->weaponBelowArm);
	render_avatar(ava->armOverHairBelowWeapon);
	render_avatar(ava->mailArmOverHairBelowWeapon);
	render_avatar(ava->armOverHair);
	render_avatar(ava->gloveBelowMailArm);
	render_avatar(ava->mailArmOverHair);
	render_avatar(ava->gloveWristBelowMailArm);
	render_avatar(ava->weaponOverArm);
	render_avatar(ava->handBelowWeapon);
	render_avatar(ava->gloveBelowWeapon);
	render_avatar(ava->gloveWristBelowWeapon);
	render_avatar(ava->shieldOverHair);
	render_avatar(ava->weaponOverHand);
	render_avatar(ava->handOverHair);
	render_avatar(ava->gloveOverHair);
	render_avatar(ava->gloveWristOverHair);
	render_avatar(ava->weaponOverGlove);
	render_avatar(ava->capeOverHead);
	render_avatar(ava->weaponWristOverGlove);
	render_avatar(ava->emotionOverBody);
	render_avatar(ava->characterStart);
	render_avatar(ava->tamingMobMid);
	render_avatar(ava->mobEquipUnderSaddle);
	render_avatar(ava->saddleFront);
	render_avatar(ava->mobEquipMid);
	render_avatar(ava->tamingMobFront);
	render_avatar(ava->mobEquipFront);
	delete tran;
}

void RenderSystem::render_video(Transform *tr, Video *vid, World &world)
{
	auto camera = world.get_components<Camera>().find(0)->second;

	auto width = (float)vid->width;
	auto heihgt = (float)vid->height;

	auto x = tr->get_position().x;
	auto y = tr->get_position().y;

	SDL_FRect rect = {x - camera->get_x(), y - camera->get_y(), width, heihgt};
	SDL_RenderCopyF(Window::get_renderer(), vid->texture, NULL, &rect);
}

void RenderSystem::render_fh(FootHold *fh, World &world)
{
	auto rl = fh->get_component<Line>();
	auto camera = world.get_components<Camera>().find(0)->second;

	SDL_RenderDrawLine(Window::get_renderer(),
					   rl->get_m().x - camera->get_x(),
					   rl->get_m().y - camera->get_y(),
					   rl->get_n().x - camera->get_x(),
					   rl->get_n().y - camera->get_y());
}
