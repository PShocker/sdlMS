#include "src/client/system/logic/chatballoon_logic_system.h"

#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/window/window.h"

#include <optional>
#include <ranges>

namespace sdlms {

bool ChatballoonLogicSystem::Run() {
  const auto now = Window::dt_now_;

  // 气泡按 destroy 时间消失。之前只处理了 NPC，玩家（自己和其他角色）
  // 的气泡会一直留在屏幕上。
  auto expire = [now](auto& balloon) {
    if (balloon.has_value() && balloon->destroy <= now) {
      balloon = std::nullopt;
    }
  };

  // self
  expire(CharacterGameInstance::self_.chatballoon);

  // other characters
  for (auto& other : CharacterGameInstance::others_ | std::views::values) {
    expire(other.g_character.chatballoon);
  }

  // npc
  for (auto& v_npc : NpcGameInstance::data_) {
    for (auto& npc : v_npc) {
      expire(npc.chatballoon);
    }
  }

  return true;
}

}  // namespace sdlms
