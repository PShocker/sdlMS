#include "skill_buff_ui_system.h"
#include "src/client/game_instance/skill_game_instance.h"
#include <cstdint>

void skill_buff_ui_system::render() {
  const auto &buffs = skill_game_instance::buf;
  for (uint8_t i = 0; i < buffs.size(); i++) {
    
  }
}