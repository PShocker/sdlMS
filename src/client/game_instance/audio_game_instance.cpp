#include "audio_game_instance.h"
#include "SDL3/SDL_audio.h"
#include "src/client/game/game_audio.h"
#include "src/common/ffmpeg/ffmpeg.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <cstddef>

void audio_game_instance::init() {
  SDL_AudioDeviceID device_id =
      SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
}

game_audio_cache &audio_game_instance::load_cache(const std::u16string &path) {
  if (!cache.contains(path)) {
    auto audio_node = wz_resource::sound->find(path);
    if (audio_node->type == wz::Type::UOL) {
      audio_node =
          static_cast<wz::Property<wz::WzUOL> *>(audio_node)->get_uol();
    }
    auto audio = static_cast<wz::Property<wz::WzSound> *>(audio_node);
    auto audio_data = audio->get_raw_data();
    cache[path] = ffmpeg::decode_audio_pcm(audio_data);
  }
  return cache.at(path);
}

void audio_game_instance::load_audio(const std::u16string &path,
                                     uint64_t delay) {
  game_audio audio{
      .path = path,
      .offset = 0,
      .delay = delay,
      .backgrnd = false,
  };
  auto spec = load_cache(path).spec;
  audio.stream = SDL_CreateAudioStream(&spec, NULL);
  SDL_BindAudioStream(device_id, audio.stream);
  audios.emplace(audio);
}

void audio_game_instance::load_backgrnd_audio(const std::u16string &path) {
  game_audio audio{
      .path = path,
      .offset = 0,
      .delay = 0,
      .backgrnd = true,
  };
  auto spec = load_cache(path).spec;
  audio.stream = SDL_CreateAudioStream(&spec, NULL);
  SDL_BindAudioStream(device_id, audio.stream);
  audios.emplace(audio);
}

void audio_game_instance::close_audio(const game_audio &audio) {
  SDL_UnbindAudioStream(audio.stream);
  SDL_DestroyAudioStream(audio.stream);
}