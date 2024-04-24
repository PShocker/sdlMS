#include "sdlms/sound.hpp"
#include "util/map_util.hpp"
#include "util/wz_util.hpp"
#include "util/ffmpeg_util.hpp"
#include "sound.hpp"

static uint8_t *audio_buf = NULL;
static uint8_t *audio_pos = NULL;
static size_t audio_size = 0;

Sound::Sound()
{
}

Sound::Sound(std::vector<uint8_t> data, int freq) : _data(data), _freq(freq)
{
}

std::optional<Sound> Sound::load_bgm(int mapId)
{
    auto _ffmpeg_util = util::FfmpegUtil::current();
    auto node = util::MapUtil::current()->load_map_node(mapId);
    node = node->find_from_path("info/bgm");
    if (node != nullptr)
    {
        auto url = dynamic_cast<wz::Property<wz::wzstring> *>(node)->get();
        url.insert(url.find('/'), u".img");
        return load(url);
    }
    return std::nullopt;
}

std::optional<Sound> Sound::load(std::u16string url)
{
    if (sound_map.contains(url))
    {
        // 从缓存中获取
        return sound_map[url];
    }

    auto node = util::WzUtil::current()->Sound->get_root()->find_from_path(url);

    if (node != nullptr)
    {
        auto _ffmpeg_util = util::FfmpegUtil::current();
        auto sound = dynamic_cast<wz::Property<wz::WzSound> *>(node);
        auto data = sound->get_raw_data();
        auto [pcm_data, freq] = _ffmpeg_util->decodeAudioToPCM(data);
        Sound s(pcm_data, freq);
        // 添加进缓存
        sound_map[url] = s;
        return s;
    }
    return std::nullopt;
}

int Sound::play(std::tuple<std::vector<uint8_t>, int> data)
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