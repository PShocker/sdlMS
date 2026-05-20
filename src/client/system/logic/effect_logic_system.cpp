#include "effect_logic_system.h"
#include "src/client/game_instance/effect_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <cstdint>
#include <ranges>

bool effect_logic_system::run_afterimage(game_effect &g_effect) {
  bool r = false;
  auto current_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::system_clock::now().time_since_epoch())
                          .count();
  if (g_effect.delay >= current_time) {
    return r;
  }
  g_effect.time += window::delta_time;
  std::u16string type = g_effect.id;
  auto hit_node = wz_resource::character->find(u"Afterimage/hit.img/" + type);
  auto index = std::to_string(g_effect.index);
  auto texture_node = hit_node->get_child(index);
  uint32_t delay = 120;
  if (texture_node->get_child(u"delay")) {
    delay =
        static_cast<wz::Property<int32_t> *>(texture_node->get_child(u"delay"))
            ->get();
  }
  if (g_effect.time > delay) {
    g_effect.index += 1;
    g_effect.time = 0;
    if (g_effect.index >= texture_node->children_count()) {
      r = true;
    }
  }
  return r;
}

void effect_logic_system::run_animate(std::vector<game_effect> &v) {
  for (auto it = v.begin(); it != v.end();) {
    bool remove = false;
    auto &g_effect = *it;
    switch (g_effect.type) {
    case game_effect::effect_type::afterimage: {
      remove = run_afterimage(g_effect);
      break;
    }
    case game_effect::effect_type::damage: {
      break;
    }
    default: {
      break;
    }
    }

    if (remove) {       // 需要删除
      it = v.erase(it); // erase 返回下一个有效的迭代器
    } else {
      ++it; // 只有在不删除时才前进
    }
  }
}

bool effect_logic_system::run() {
  for (auto &ce : effect_game_instance::effect | std::views::values) {
    run_animate(ce);
  }
  for (auto &me : effect_game_instance::effect2 | std::views::values) {
    run_animate(me);
  }
  for (auto &de : effect_game_instance::data) {
    run_animate(de);
  }
  return true;
}