#include "chatballoon_logic_system.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/window/window.h"
#include <optional>

bool chatballoon_logic_system::run() {
  // self
  auto &self = character_game_instance::self;
  if (self.chatballoon.has_value()) {
    if (self.chatballoon.value().destory <= window::dt_now) {
      self.chatballoon = std::nullopt;
    }
  }

  // npc
  for (auto &v_npc : npc_game_instance::data) {
    for (auto &npc : v_npc) {
      if (!npc.chatballoon.has_value()) {
        continue;
      }
      if (npc.chatballoon.value().destory <= window::dt_now) {
        npc.chatballoon = std::nullopt;
      }
    }
  }

  return true;
}