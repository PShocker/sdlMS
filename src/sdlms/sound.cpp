#include "sdlms/sound.hpp"

static uint8_t *audio_buf = NULL;
static uint8_t *audio_pos = NULL;
static size_t audio_size = 0;

Sound::Sound()
{
}

int Sound::load(std::tuple<std::vector<uint8_t>, int> data)
{
    auto [pcm_data, freq] = data;

    audio_size = pcm_data.size();
    audio_buf = new uint8_t[pcm_data.size()];
    audio_pos = audio_buf;
    memcpy(audio_buf, pcm_data.data(), pcm_data.size());

    SDL_AudioSpec spec;
    spec.freq = freq;
    spec.format = AUDIO_S16SYS;
    spec.channels = 2;
    spec.silence = 0;
    spec.samples = 1024;
    spec.callback = [](void *udata, Uint8 *stream, int len) -> void
    {
        if (audio_size <= 0)
        {
            return;
        }
        len = (len < audio_size) ? len : audio_size;

        SDL_memset(stream, 0, len);

        SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);

        audio_pos += len;
        audio_size -= len;
    };
    spec.userdata = NULL;

    // open audio devcie
    if (SDL_OpenAudio(&spec, NULL))
    {
        return -1;
    }
    // play audio
    SDL_PauseAudio(0);
    return 0;
}