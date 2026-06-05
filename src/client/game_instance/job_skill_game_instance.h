#pragma once

#include <cstdint>
#include <flat_map>
#include <string>
class job_skill_game_instance {
private:
  static void load_job();
  static void load_skill_point();

public:
  static void load();
  static uint8_t load_self_skill_level(const std::u16string &id);

  static inline std::flat_map<uint32_t, uint8_t> self_skill_point;
};