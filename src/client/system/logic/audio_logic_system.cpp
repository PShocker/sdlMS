#include "audio_logic_system.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/window/window.h"

void AudioLogicSystem::RunBackgrnd() {
  auto &backgrnds = AudioGameInstance::backgrnds;
  if (backgrnds.empty()) {
    return;
  }
  auto &audio = backgrnds[0];
  auto &data = AudioGameInstance::LoadCache(audio.path);
  if (SDL_GetAudioStreamQueued(audio.stream) < (int)data.data.size()) {
    // 当数据不足时，重新放入完整音频数据
    SDL_PutAudioStreamData(audio.stream, data.data.data(), data.data.size());
  }
  if (audio.fade > 0) {
    audio.fade -= 0.010;
    SDL_SetAudioStreamGain(audio.stream, audio.fade);
  } else if (backgrnds.size() > 1) {
    AudioGameInstance::CloseAudio(audio);
    backgrnds.erase(backgrnds.begin());
  }
}

void AudioLogicSystem::RunAudios() {
  auto now = Window::dt_time;
  auto &audios = AudioGameInstance::audios;
  for (auto it = audios.begin(); it != audios.end();) {
    if (now >= it->delay || it->delay == 0) {
      if (it->fade == 0) {
        auto &data = AudioGameInstance::LoadCache(it->path);
        SDL_PutAudioStreamData(it->stream, data.data.data(), data.data.size());
        SDL_FlushAudioStream(it->stream);
        it->fade = 1;
      } else if (SDL_GetAudioStreamQueued(it->stream) == 0) {
        AudioGameInstance::CloseAudio(*it);
        it = audios.erase(it);
        continue;
      }
    }
    ++it;
  }
}

bool AudioLogicSystem::Run() {
  RunBackgrnd();
  RunAudios();
  return true;
}