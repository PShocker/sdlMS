#pragma once

#include "src/client/game/game_audio.h"
#include <cstdint>
#include <vector>
class Ffmpeg {
public:
  static GameAudioCache
  decode_audio_pcm(const std::vector<uint8_t> &audio_data);
};