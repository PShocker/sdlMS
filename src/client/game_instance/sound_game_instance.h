#pragma once
#include "SDL3/SDL_audio.h"
#include "src/client/game/game_sound.h"
#include <vector>
class sound_game_instance {
public:
  static void load_map_sound(uint32_t map_id);

  static inline SDL_AudioStream *audio_stream;
  static inline game_sound backgrnd_sound;
  static inline std::vector<game_sound> data;
};