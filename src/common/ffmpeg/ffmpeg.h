#pragma once

#include "src/client/game/game_audio.h"
#include <cstdint>
#include <vector>
class ffmpeg {
public:
  static game_audio_cache
  decode_audio_pcm(const std::vector<uint8_t> &audio_data);
};