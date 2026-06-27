#pragma once

#include "SDL3/SDL_audio.h"
#include <cstdint>
#include <string>
#include <vector>

struct game_audio_cache {
  SDL_AudioSpec spec;
  std::vector<uint8_t> data;
};

class game_audio {
public:
  std::u16string path;
  uint64_t delay;
  float fade;
  SDL_AudioStream *stream;
};