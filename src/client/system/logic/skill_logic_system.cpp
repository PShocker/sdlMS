#include "skill_logic_system.h"
#include "src/client/game_instance/skill_game_instance.h"

bool skill_logic_system::run() {
  auto &v = skill_game_instance::ski;
  for (auto it = v.begin(); it != v.end();) {
    if (!it->frame) {
      it++;
      continue;
    }
    auto remove = it->frame();
    if (remove) {       // 需要删除
      it = v.erase(it); // erase 返回下一个有效的迭代器
    } else {
      ++it; // 只有在不删除时才前进
    }
  }
  return true;
}