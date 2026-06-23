#include "job_skill_game_instance.h"
#include "character_game_instance.h"
#include <cstdint>

void job_skill_game_instance::load(const character_save &cs) {
  skill_point = cs.sp.ski_sp;
}

uint8_t job_skill_game_instance::load_skill_level(const std::u16string &id) {
  auto ski_id = std::stoi(std::string(id.begin(), id.end()));
  if (skill_point.contains(ski_id)) {
    return skill_point.at(ski_id);
  } else {
    return 0;
  }
}