#pragma once
#include "SDL3/SDL_audio.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_buff.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_skill.h"
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
  static auto &skis() {
    static std::flat_map<std::u16string, game_skill> instance;
    return instance;
  }

  static wz::Node *load_ski_node(uint32_t id);
  static wz::Node *load_ski_node(const std::string &id);
  static wz::Node *load_ski_node(const std::u16string &id);
  static wz::Node *load_ski_level_node(const std::u16string &id, uint8_t l);

  static uint64_t load_ski_time(game_character &g_character);

  struct skill_name {
    std::u16string name;
    std::u16string desc;
    std::vector<std::u16string> level;
  };
  static skill_name load_ski_name(const std::u16string &id);
  static int load_ski_max_lvl(const std::u16string &id);
  static bool load_ski_active(const std::u16string &id);
};