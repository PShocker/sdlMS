#include "character_logic_system.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/window/window.h"
#include <cstdint>

// 人物状态机
bool character_logic_system::run() {
  auto &self = character_game_instance::self;
  auto delta = window::delta_time;
  uint32_t size = 0;
  self.action_time += delta;
  //   获取动作延迟时间
  uint32_t delay;
  if (character_game_instance::extern_action.contains(self.action)) {
    auto &action_info = character_game_instance::extern_action.at(self.action);
    delay = action_info[self.action_index].delay;
    size = action_info.size();
  } else {
    auto &action_info = character_game_instance::bone_data.at(self.action);
    delay = action_info[self.action_index].delay;
    size = action_info.size();
  }
  if (self.action_time >= delay) {
    self.action_index += 1;
    self.action_index = self.action_index % size;
    self.action_time = 0;
  }

  return true;
}