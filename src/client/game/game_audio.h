#pragma once

#include "SDL3/SDL_audio.h"
#include <cstdint>
#include <string>
#include <vector>

struct GameAudioCache {
  SDL_AudioSpec spec;
  std::vector<uint8_t> data;
};

struct GameAudio{
  std::u16string path;
  uint64_t delay;
  float fade;
  SDL_AudioStream *stream;
  float volume;
};