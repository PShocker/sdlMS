#pragma once

#include "SDL3/SDL_audio.h"
#include "src/client/game/game_audio.h"
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <string>
#include <vector>

class audio_game_instance {
public:
  static inline std::vector<game_audio> audios;
  static game_audio_cache &load_cache(const std::u16string &path);

  static void load_audio(const std::u16string &path, uint64_t delay);
  static void load_backgrnd_audio(const std::u16string &path);

  static void close_audio(const game_audio &audio);

  static void init();
  static inline SDL_AudioDeviceID device_id;
  static inline std::flat_map<std::u16string, game_audio_cache> cache;
};