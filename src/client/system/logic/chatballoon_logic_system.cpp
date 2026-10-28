#include "src/client/system/logic/chatballoon_logic_system.h"

#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/window/window.h"

#include <optional>
namespace sdlms {


bool ChatballoonLogicSystem::Run() {
  // self
  auto& self = CharacterGameInstance::self_;
  if (self.chatballoon.has_value()) {
    if (self.chatballoon.value().destroy <= Window::dt_now_) {
      // self.chatballoon = std::nullopt;
    }
  }

  // npc
  for (auto& v_npc : NpcGameInstance::data_) {
    for (auto& npc : v_npc) {
      if (!npc.chatballoon.has_value()) {
        continue;
      }
      if (npc.chatballoon.value().destroy <= Window::dt_now_) {
        npc.chatballoon = std::nullopt;
      }
    }
  }

  return true;
}

}  // namespace sdlms
