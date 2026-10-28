#pragma once

#include "src/client/game/game_save.h"
#include <cstdint>
#include <flat_map>
#include <string>
#include <vector>

enum class JobType : uint8_t {
  kBeginner,    // 000
  kWarrior,     // 100
  kMagician,    // 200
  kBowman,      // 300
  kThief,       // 400
  kFighter,     // 110
  kPage,        // 120
  kSpearman,    // 130
  kFpWizard,   // 210
  kIlWizard,   // 220
  kCleric,      // 230
  kHunter,      // 310
  kCrossbowman, // 320
  kAssassin,    // 410
  kBandit,      // 420
};

class JobSkillGameInstance {
public:
  static void load(const CharacterSaveData &cs);

  static uint8_t load_ski_level(const std::u16string &id);
  static std::vector<JobType> load_ski_tree(const std::u16string &id);
  static std::vector<JobType> load_ski_tree(int i);
  static std::u16string load_job_id(JobType type);
  static wz::WzMap load_job_skis(JobType type);

  static std::u16string load_two_jump();

  static inline std::flat_map<uint32_t, uint8_t> skill_point;
  static inline std::vector<uint32_t> remain_point = {100, 100, 100, 100};
};