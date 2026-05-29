#pragma once
#include "SDL3/SDL_audio.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_buff.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_sound.h"
#include "wz/Node.h"
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <optional>
#include <string>
#include <vector>

class skill_game_instance {
public:
  static inline std::vector<game_buff> buf;

  static wz::Node *load_skill_node(const std::u16string &id);
  static wz::Node *load_skill_level_node(const std::u16string &id, uint8_t l);
  enum skill_type {
    attack,
    move,
    buff,
  };
  static skill_type load_skill_type(const std::u16string &id, uint8_t l);
  static SDL_FRect load_skill_rect(const std::u16string &id, uint8_t l);
  static uint64_t load_beat_time(game_character &g_character);
};