#include "skill_logic_system.h"
#include "src/client/game_instance/skill_game_instance.h"

bool skill_logic_system::run() {
  auto &v = skill_game_instance::ski;
  for (const auto &sk : v) {
    sk.frame();
  }
  return true;
}