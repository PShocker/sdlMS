#include "SoundSystem.h"
#include <SDL2/SDL.h>

void SoundSystem::run(World &world)
{
	if (world.components_exist_of_type<Sound>())
	{
		for (auto &[index, sou] : world.get_components<Sound>())
		{
			if (!sou->get_play())
			{
				play_sound(sou);
				// SDL_CreateThread(SoundSystem::play_sound, "AudioThread", (void *)sou);
				sou->set_play(true);
			}
		}
	}
}

int SoundSystem::play_sound(void *sou)
{
	Sound *sound = (Sound *)sou;

	auto data = sound->get_pcm_data();
	unsigned int data_size = data->size();

	SDL_AudioSpec spec;
	spec.freq = sound->get_freq();
	spec.format = AUDIO_S16SYS;
	spec.channels = 2;
	spec.silence = 0;
	spec.samples = 1024;
	spec.callback = [](void *udata, Uint8 *stream, int len) -> void
	{
		auto sou = (Sound *)(udata);
		auto data = sou->get_pcm_data();
		auto offset = sou->get_offset();
		len = std::min((unsigned int)data->size() - offset, (unsigned int)len);

		SDL_memcpy(stream, data->data() + offset, len);
		offset += len;
		// 如果到达数据末尾，则重置播放位置以循环播放
		if (offset >= data->size())
		{
			if (sou->get_delay() == -1)
			{
				SDL_PauseAudio(1);
			}
			offset = 0;
		}
		sou->set_offset(offset);
	};
	spec.userdata = sou;

	// open audio devcie
	if (SDL_OpenAudio(&spec, NULL))
	{
		return -1;
	}
	// play audio
	SDL_PauseAudio(0);
	return 0;
}
