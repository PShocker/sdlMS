#include "audio_logic_system.h"
#include "src/client/game_instance/audio_game_instance.h"
#include <chrono>

bool audio_logic_system::run() {
  auto &audios = audio_game_instance::audios;
  auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                 std::chrono::system_clock::now().time_since_epoch())
                 .count();
  for (auto it = audios.begin(); it != audios.end();) {
    auto &data = audio_game_instance::load_cache(it->path);
    if (it->backgrnd) {
      if (SDL_GetAudioStreamQueued(it->stream) < (int)data.data.size()) {
        // 当数据不足时，重新放入完整音频数据
        SDL_PutAudioStreamData(it->stream, data.data.data(), data.data.size());
      }
    } else {
      if (now >= it->delay || it->delay == 0) {
        if (it->offset == 0) {
          SDL_PutAudioStreamData(it->stream, data.data.data(),
                                 data.data.size());
          SDL_FlushAudioStream(it->stream);
          it->offset = data.data.size();
        } else if (SDL_GetAudioStreamQueued(it->stream) == 0) {
          audio_game_instance::close_audio(*it);
          it = audios.erase(it);
          continue;
        }
      }
    }
    it++;
  }
  return true;
}