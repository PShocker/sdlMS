#ifndef SDLMS_SRC_CLIENT_GAME_INSTANCE_JOB_SKILL_GAME_INSTANCE_H_
#define SDLMS_SRC_CLIENT_GAME_INSTANCE_JOB_SKILL_GAME_INSTANCE_H_

#include "src/client/game/game_save.h"

#include <cstdint>
#include <flat_map>
#include <string>
#include <vector>
namespace sdlms {


enum class JobType : uint8_t {
  kBeginner,     // 000
  kWarrior,      // 100
  kMagician,     // 200
  kBowman,       // 300
  kThief,        // 400
  kFighter,      // 110
  kPage,         // 120
  kSpearman,     // 130
  kFpWizard,     // 210
  kIlWizard,     // 220
  kCleric,       // 230
  kHunter,       // 310
  kCrossbowman,  // 320
  kAssassin,     // 410
  kBandit,       // 420
};

class JobSkillGameInstance {
 public:
  static void Load(const CharacterSaveData& cs);

  static uint8_t LoadSkiLevel(const std::u16string& id);
  static std::vector<JobType> LoadSkiTree(const std::u16string& id);
  static std::vector<JobType> LoadSkiTree(int i);
  static std::u16string LoadJobId(JobType type);
  static wz::WzMap LoadJobSkis(JobType type);

  static std::u16string LoadTwoJump();

  static inline std::flat_map<uint32_t, uint8_t> skill_point_;
  static inline std::vector<uint32_t> remain_point = {100, 100, 100, 100};
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_INSTANCE_JOB_SKILL_GAME_INSTANCE_H_
