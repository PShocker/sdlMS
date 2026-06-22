#pragma once

#include <cstdint>
#include <flat_map>
#include <string>
class job_skill_game_instance {
public:
  static uint8_t load_skill_level(const std::u16string &id);

  static inline std::flat_map<uint32_t, uint8_t> skill_point;
};