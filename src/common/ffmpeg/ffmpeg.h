#pragma once

#include "src/client/game/game_audio.h"
#include <cstdint>
#include <vector>
namespace sdlms {

class Ffmpeg {
public:
  static GameAudioCache
  DecodeAudioPcm(const std::vector<uint8_t> &audio_data);
};

}  // namespace sdlms
