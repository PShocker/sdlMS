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
  static inline float backgrnd_volume_;
  static inline float audio_volume_;

  static inline std::vector<GameAudio> backgrnds_;
  static inline std::vector<GameAudio> audios_;
  static GameAudioCache &LoadCache(const std::u16string &path);

  static void LoadAudio(const std::u16string &path, uint64_t delay);
  static void LoadBackgrndAudio(const std::u16string &path);

  static void CloseAudio(const GameAudio &audio);

  static void Init();
  static inline SDL_AudioDeviceID device_id_;
  static inline std::flat_map<std::u16string, GameAudioCache> cache_;
};