#include "audio_game_instance.h"
#include "SDL3/SDL_audio.h"
#include "src/client/game/game_audio.h"
#include "src/common/ffmpeg/ffmpeg.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <cstddef>

void audio_game_instance::init() {
  device_id = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
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
  // game_audio audio{
  //     .path = path,
  //     .delay = delay,
  //     .fade = 0,
  // };
  // auto spec = load_cache(path).spec;
  // audio.stream = SDL_CreateAudioStream(&spec, NULL);
  // SDL_BindAudioStream(device_id, audio.stream);
  // audios.emplace_back(audio);
}

void audio_game_instance::load_backgrnd_audio(const std::u16string &path) {
  // if (path.empty()) {
  //   return;
  // }
  // if (!backgrnds.empty()) {
  //   if (backgrnds[0].path == path && backgrnds[0].fade == 0) {
  //     return;
  //   }
  // }
  // game_audio backgrnd{
  //     .path = path,
  //     .delay = 0,
  //     .fade = 0,
  // };
  // auto spec = load_cache(path).spec;
  // backgrnd.stream = SDL_CreateAudioStream(&spec, NULL);
  // SDL_BindAudioStream(device_id, backgrnd.stream);
  // if (!backgrnds.empty()) {
  //   if (backgrnds[0].fade == 0) {
  //     backgrnds[0].fade = 1;
  //   }
  // }
  // backgrnds.emplace_back(backgrnd);
}

void audio_game_instance::close_audio(const game_audio &audio) {
  SDL_UnbindAudioStream(audio.stream);
  SDL_DestroyAudioStream(audio.stream);
}