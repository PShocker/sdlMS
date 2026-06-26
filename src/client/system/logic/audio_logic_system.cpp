#include "audio_logic_system.h"
#include "src/client/game_instance/audio_game_instance.h"
#include <chrono>

bool audio_logic_system::run() {
  auto &audios = audio_game_instance::audios;
  auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                 std::chrono::system_clock::now().time_since_epoch())
                 .count();
  for (auto it = audios.begin(); it != audios.end();) {
    auto data = audio_game_instance::load_cache(it->path);
    if (it->backgrnd) {

    } else {
      if (now >= it->delay || it->delay == 0) {
        SDL_PutAudioStreamData(it->stream, data.data.data(), data.data.size());
        audio_game_instance::close_audio(*it);
        it = audios.erase(it);
        continue;
      }
    }
    it++;
  }
  return true;
}