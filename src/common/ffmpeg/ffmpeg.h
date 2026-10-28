#ifndef SDLMS_SRC_COMMON_FFMPEG_FFMPEG_H_
#define SDLMS_SRC_COMMON_FFMPEG_FFMPEG_H_

#include "src/client/game/game_audio.h"

#include <cstdint>
#include <vector>
namespace sdlms {

class Ffmpeg {
 public:
  static GameAudioCache DecodeAudioPcm(const std::vector<uint8_t>& audio_data);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_COMMON_FFMPEG_FFMPEG_H_
