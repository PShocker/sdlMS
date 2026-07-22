#include "audio_logic_system.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/window/window.h"

void audio_logic_system::run_backgrnd() {
  auto &backgrnds = audio_game_instance::backgrnds;
  if (backgrnds.empty()) {
    return;
  }
  auto &audio = backgrnds[0];
  auto &data = audio_game_instance::load_cache(audio.path);
  if (SDL_GetAudioStreamQueued(audio.stream) < (int)data.data.size()) {
    // 当数据不足时，重新放入完整音频数据
    SDL_PutAudioStreamData(audio.stream, data.data.data(), data.data.size());
  }
  if (audio.fade > 0) {
    audio.fade -= 0.010;
    SDL_SetAudioStreamGain(audio.stream, audio.fade);
  } else if (backgrnds.size() > 1) {
    audio_game_instance::close_audio(audio);
    backgrnds.erase(backgrnds.begin());
  }
}

void audio_logic_system::run_audios() {
  auto now = window::dt_time;
  auto &audios = audio_game_instance::audios;
  for (auto it = audios.begin(); it != audios.end();) {
    if (now >= it->delay || it->delay == 0) {
      if (it->fade == 0) {
        auto &data = audio_game_instance::load_cache(it->path);
        SDL_PutAudioStreamData(it->stream, data.data.data(), data.data.size());
        SDL_FlushAudioStream(it->stream);
        it->fade = 1;
      } else if (SDL_GetAudioStreamQueued(it->stream) == 0) {
        audio_game_instance::close_audio(*it);
        it = audios.erase(it);
        continue;
      }
    }
    ++it;
  }
}

bool audio_logic_system::run() {
  run_backgrnd();
  run_audios();
  return true;
}