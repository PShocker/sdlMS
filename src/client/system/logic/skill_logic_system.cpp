#include "skill_logic_system.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/window/window.h"
#include <functional>
#include <vector>

bool skill_logic_system::run() {
  auto &v = skill_game_instance::ski;
  std::vector<std::function<void()>> ends;
  for (const auto &sk : v) {
    if (sk.destory <= window::dt_now) {
      ends.push_back(sk.end);
    }
  }
  for (auto fn : ends) {
    fn();
  }
  for (const auto &sk : v) {
    if (sk.frame) {
      sk.frame();
    }
  }
  return true;
}