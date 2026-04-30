#include "character_logic_system.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/window/window.h"
#include <cstdint>

// 人物状态机
bool character_logic_system::run() {
  auto &self = character_game_instance::self;
  auto delta = window::delta_time;
  self.action_time += delta;
  //   获取动作延迟时间
  uint32_t delay;
  if (character_game_instance::extern_action.contains(self.action)) {
    delay = character_game_instance::extern_action
                .at(self.action)[self.action_index]
                .delay;
  } else {
    delay =
        character_game_instance::bone_data.at(self.action)[self.action_index]
            .delay;
  }
  //   if (self.action_index >= delay) {
  //     self.action_index += 1;
  //   }

  return true;
}

void character_logic_system::input_action(game_input &g_input) {
  auto &self = character_game_instance::self;
  if (g_input.val == "left" || g_input.val == "right") {
    //  左右
    switch (character_game_instance::self_state) {
    case character_game_instance::state::STAND:
    case character_game_instance::state::WALK: {
      if (g_input.val == "left") {
        self.flip = 0;
        character_game_instance::self_hforce = -1400;
      } else {
        self.flip = 1;
        character_game_instance::self_hforce = 1400;
      }
      break;
    }

    default: {
      break;
    }
    }
  }
}