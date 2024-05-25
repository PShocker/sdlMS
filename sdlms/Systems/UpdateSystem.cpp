#include "UpdateSystem.h"

#include <SDL2/SDL.h>

void UpdateSystem::run(World &world)
{
	if (world.components_exist_of_type<AnimatedSprite>())
	{
		for (auto &[index, aspr] : world.get_components<AnimatedSprite>())
		{
			if (aspr->get_owner() != nullptr)
			{
				update_animated_sprite(aspr, world);
			}
		}
	}
	if (world.components_exist_of_type<Avatar>())
	{
		for (auto &[index, ava] : world.get_components<Avatar>())
		{
			update_avatar(ava, ava->get_owner_component<Transform>(), world);
		}
	}
	if (world.components_exist_of_type<Video>())
	{
		for (auto &[index, vid] : world.get_components<Video>())
		{
			// SDL_CreateThread(UpdateSystem::update_video, "decodeThread", (void *)vid);
			update_video(vid);
		}
	}
}

bool UpdateSystem::update_animated_sprite(AnimatedSprite *aspr, World &world)
{
	bool end = false;
	auto delta_time = world.get_delta_time();
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
	return end;
}

void UpdateSystem::update_avatar(Avatar *ava, Transform *tr, World &world)
{
	[[likely]]
	if (ava->animate)
	{
		auto delta_time = world.get_delta_time();
		auto delay = ava->stance_delays[ava->act][ava->act_index];
		ava->act_time += delta_time;
		if (ava->act_time >= delay)
		{
			ava->act_index = (ava->act_index + 1) % ava->stance_delays[ava->act].size();
			ava->act_time = 0;
		}
	}
}

int UpdateSystem::update_video(void *vid)
{
	// 解码并显示视频帧
	Video *video = (Video *)vid;
	AVPacket packet;
	AVFrame *frame = av_frame_alloc();
	while (av_read_frame(video->formatContext, &packet) >= 0)
	{
		if (packet.stream_index == video->videoStreamIndex)
		{
			avcodec_send_packet(video->codecContext, &packet);
			if (avcodec_receive_frame(video->codecContext, frame) == 0)
			{
				SDL_UpdateYUVTexture(video->texture, NULL,
									 frame->data[0], frame->linesize[0],
									 frame->data[1], frame->linesize[1],
									 frame->data[2], frame->linesize[2]);
				av_packet_unref(&packet);
				break;
			}
		}
		av_packet_unref(&packet);
	}
	av_frame_free(&frame);
	return 0;
}
