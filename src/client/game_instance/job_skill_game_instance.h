#pragma once

#include "src/client/game/game_save.h"
#include <cstdint>
#include <flat_map>
#include <string>
#include <vector>

enum class job_type : uint8_t {
  BEGINNER,    // 000
  WARRIOR,     // 100
  MAGICIAN,    // 200
  BOWMAN,      // 300
  THIEF,       // 400
  FIGHTER,     // 110
  PAGE,        // 120
  SPEARMAN,    // 130
  FP_WIZARD,   // 210
  IL_WIZARD,   // 220
  CLERIC,      // 230
  HUNTER,      // 310
  CROSSBOWMAN, // 320
  ASSASSIN,    // 410
  BANDIT,      // 420
};

class job_skill_game_instance {
public:
  static void load(const character_save &cs);

  static void load_passive_ski();

  static uint8_t load_ski_level(const std::u16string &id);
  static std::vector<job_type> load_ski_tree(const std::u16string &id);
  static std::vector<job_type> load_ski_tree(int i);
  static std::u16string load_job_id(job_type type);
  static wz::WzMap load_job_skis(job_type type);

  static inline std::flat_map<uint32_t, uint8_t> skill_point;
  static inline std::vector<uint32_t> remain_point = {100, 100, 100, 100};
};