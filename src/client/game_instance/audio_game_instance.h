#pragma once

#include "src/client/game/game_audio.h"

#include <SDL3/SDL_audio.h>

#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <optional>
#include <string>
#include <vector>

namespace sdlms {

class AudioGameInstance {
 public:
  // 选项窗口的音量（0..1）。master 与各通道分开保存，播放时相乘。
  static inline float master_volume_ = 1.0f;
  static inline float bgm_volume_ = 1.0f;
  static inline float sfx_volume_ = 1.0f;

  static inline std::vector<GameAudio> backgrnds_;
  static inline std::vector<GameAudio> audios_;
  static GameAudioCache& LoadCache(const std::u16string& path);

  // 设置音量，并立即作用到正在播放的音频流上
  static void SetVolume(float master, float bgm, float sfx);
  static float SfxGain();
  static float BgmGain();

  static void LoadAudio(const std::u16string& path, uint64_t delay);
  static void LoadBackgrndAudio(const std::u16string& path);

  static void CloseAudio(const GameAudio& audio);

  static void Init();
  static inline SDL_AudioDeviceID device_id_;
  static inline std::flat_map<std::u16string, GameAudioCache> cache_;
};

}  // namespace sdlms
