#include "effect_logic_system.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/effect_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
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

bool effect_logic_system::run_skill_use(game_effect &g_effect) {
  bool r = false;
  auto ski_node = skill_game_instance::load_skill_node(g_effect.id);
  ski_node = ski_node->get_child(u"effect");
  auto index = std::to_string(g_effect.index);
  auto texture_node = ski_node->get_child(index);
  auto delay_node = texture_node->get_child(u"delay");
  int32_t delay = 100;
  if (delay_node) {
    delay = static_cast<wz::Property<std::int32_t> *>(delay_node)->get();
  }
  g_effect.time += window::delta_time;
  if (g_effect.time >= delay) {
    g_effect.time = 0;
    g_effect.index += 1;
    auto count = ski_node->children_count();
    r = g_effect.index >= count;
    g_effect.index = g_effect.index % count;
  }
  return r;
}

bool effect_logic_system::run_skill_hit(game_effect &g_effect) {
  auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                 std::chrono::system_clock::now().time_since_epoch())
                 .count();
  if (g_effect.delay >= now) {
    return false;
  }
  bool r = false;
  auto ski_node = skill_game_instance::load_skill_node(g_effect.id);
  ski_node = ski_node->get_child(u"hit")->get_child(u"0");
  auto index = std::to_string(g_effect.index);
  auto texture_node = ski_node->get_child(index);
  auto delay_node = texture_node->get_child(u"delay");
  int32_t delay = 100;
  if (delay_node) {
    delay = static_cast<wz::Property<std::int32_t> *>(delay_node)->get();
  }
  g_effect.time += window::delta_time;
  if (g_effect.time >= delay) {
    g_effect.time = 0;
    g_effect.index += 1;
    auto count = ski_node->children_count();
    r = g_effect.index >= count;
    g_effect.index = g_effect.index % count;
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
    case game_effect::effect_type::skill_use: {
      remove = run_skill_use(g_effect);
      break;
    }
    case game_effect::effect_type::skill_hit: {
      remove = run_skill_hit(g_effect);
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
  run_animate(character_game_instance::self.effect);
  for (auto &o : character_game_instance::others | std::views::values) {
    run_animate(o.g_character.effect);
  }
  for (auto &m : mob_game_instance::data | std::views::values) {
    run_animate(m.mob.effect);
  }
  for (auto &de : effect_game_instance::data) {
    run_animate(de);
  }
  return true;
}