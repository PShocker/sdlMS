#pragma once

#include "SDL3/SDL_audio.h"
#include "src/client/game/game_audio.h"
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <optional>
#include <string>
#include <vector>

class AudioGameInstance {
public:
  static inline float backgrnd_volume;
  static inline float audio_volume;

  static inline std::vector<GameAudio> backgrnds;
  static inline std::vector<GameAudio> audios;
  static GameAudioCache &load_cache(const std::u16string &path);

  static void load_audio(const std::u16string &path, uint64_t delay);
  static void load_backgrnd_audio(const std::u16string &path);

  static void close_audio(const GameAudio &audio);

  static void init();
  static inline SDL_AudioDeviceID device_id;
  static inline std::flat_map<std::u16string, GameAudioCache> cache;
};