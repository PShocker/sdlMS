#include "audio_game_instance.h"
#include "SDL3/SDL_audio.h"
#include "src/client/game/game_audio.h"
#include "src/common/ffmpeg/ffmpeg.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <cstddef>

void AudioGameInstance::Init() {
  device_id_ = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
}

GameAudioCache &AudioGameInstance::LoadCache(const std::u16string &path) {
  if (!cache_.contains(path)) {
    auto audio_node = WzResource::sound_->find(path);
    if (audio_node->type == wz::Type::UOL) {
      audio_node =
          static_cast<wz::Property<wz::WzUOL> *>(audio_node)->get_uol();
    }
    auto audio = static_cast<wz::Property<wz::WzSound> *>(audio_node);
    auto audio_data = audio->get_raw_data();
    cache_[path] = Ffmpeg::DecodeAudioPcm(audio_data);
  }
  return cache_.at(path);
}

void AudioGameInstance::LoadAudio(const std::u16string &path,
                                     uint64_t delay) {
  GameAudio audio{
      .path = path,
      .delay = delay,
      .fade = 0,
  };
  auto spec = LoadCache(path).spec;
  audio.stream = SDL_CreateAudioStream(&spec, NULL);
  SDL_BindAudioStream(device_id_, audio.stream);
  audios_.emplace_back(audio);
}

void AudioGameInstance::LoadBackgrndAudio(const std::u16string &path) {
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

void AudioGameInstance::CloseAudio(const GameAudio &audio) {
  SDL_UnbindAudioStream(audio.stream);
  SDL_DestroyAudioStream(audio.stream);
}