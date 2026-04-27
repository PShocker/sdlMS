#pragma once

#include <cstdint>
#include <flat_map>
class job_skill_game_instance {
private:
  static void load_job();
  static void load_skill_point();

public:
  static void load();

  static inline uint32_t self_job = 130;
  static inline std::flat_map<uint32_t, uint8_t> self_skill_point;
};