#include "src/client/system/logic/skill_logic_system.h"

#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/window/window.h"

#include <functional>
#include <vector>

namespace sdlms {

bool SkillLogicSystem::Run() {
  auto& v = SkillGameInstance::ski_;
  std::vector<std::function<void()>> ends;
  for (const auto& sk : v) {
    if (sk.destroy <= Window::dt_now_ && sk.destroy != 0) {
      ends.push_back(sk.end);
    }
  }
  for (auto fn : ends) {
    fn();
  }
  std::vector<std::function<void()>> frames;
  for (const auto& sk : v) {
    if (sk.frame) {
      frames.push_back(sk.frame);
    }
  }
  for (auto& fn : frames) {
    fn();
  }
  return true;
}

}  // namespace sdlms
