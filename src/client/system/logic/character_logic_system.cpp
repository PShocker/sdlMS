#include "character_logic_system.h"
#include "SDL3/SDL_rect.h"
#include "drop_logic_system.h"
#include "src/client/game/game_character.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/game_instance/ladderrope_game_instance.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/physic/physic.h"
#include "src/common/request/client_request.h"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <flat_map>
#include <flat_set>
#include <ranges>
#include <string>

bool character_logic_system::run_action(game_character &g_character,
                                        const std::u16string &action) {
  if (g_character.action == action) {
    return false;
  }
  g_character.action = action;
  g_character.action_index = 0;
  g_character.action_time = 0;
  return true;
}

void character_logic_system::run_walk_action(game_character &g_character) {
  if (g_character.weapon.has_value()) {
    auto weapon_id = g_character.weapon.value().id;
    const auto &avatar_data = character_game_instance::avatar_data;
    if (avatar_data.at(weapon_id).data.contains(u"walk1")) {
      run_action(g_character, u"walk1");
    } else {
      run_action(g_character, u"walk2");
    }
  } else {
    run_action(g_character, u"walk1");
  }
  return;
}

void character_logic_system::run_stand_action(game_character &g_character) {
  if (g_character.weapon.has_value()) {
    auto weapon_id = g_character.weapon.value().id;
    const auto &avatar_data = character_game_instance::avatar_data;
    if (avatar_data.at(weapon_id).data.contains(u"stand1")) {
      run_action(g_character, u"stand1");
    } else {
      run_action(g_character, u"stand2");
    }
  } else {
    run_action(g_character, u"stand1");
  }
  return;
}

bool character_logic_system::run_animate(game_character &g_character) {
  bool r = false;
  auto delta = window::delta_time;
  uint32_t size = 0;
  g_character.action_time += delta;
  //   获取动作延迟时间
  uint32_t delay;
  if (character_game_instance::extern_action.contains(g_character.action)) {
    auto &action_info =
        character_game_instance::extern_action.at(g_character.action);
    delay = action_info[g_character.action_index].delay;
    size = action_info.size();
  } else {
    auto &action_info =
        character_game_instance::bone_data.at(g_character.action);
    delay = action_info[g_character.action_index].delay;
    size = action_info.size();
  }
  if (g_character.action_time >= delay) {
    g_character.action_index += 1;
    r = g_character.action_index > size;
    g_character.action_index = g_character.action_index % size;
    g_character.action_time = 0;
  }
  return r;
}

bool character_logic_system::run_flip(game_character &g_character) {
  if (g_character.abnormals.contains(
          game_character::abnormal_state_type::dizz)) {
    return false;
  }
  if (character_action_input.contains("left")) {
    g_character.flip = 0;
    return true;
  }
  if (character_action_input.contains("right")) {
    g_character.flip = 1;
    return true;
  }
  return false;
}

void character_logic_system::run_pick(game_character &g_character) {
  if (g_character.abnormals.contains(
          game_character::abnormal_state_type::dizz)) {
    return;
  }
  if (character_action_input.contains("pick")) {
    drop_logic_system::pick(g_character);
    return;
  }
}

bool character_logic_system::run_walk(game_character &g_character) {
  bool r = true;
  if (g_character.abnormals.contains(
          game_character::abnormal_state_type::dizz)) {
    return r;
  }
  if (character_action_input.contains("left")) {
    self_hforce = -1400;
    run_walk_action(g_character);
  } else if (character_action_input.contains("right")) {
    self_hforce = 1400;
    run_walk_action(g_character);
  } else {
    // 左右都没输入，stand或alert
    if (self_alert_cooldown > window::dt_now) {
      run_action(g_character, u"alert");
    } else {
      run_stand_action(g_character);
    }
    self_hforce = 0;
    if (self_hspeed == 0) {
      return r;
    }
  }
  // 移动
  auto delta_time = window::delta_time / 1000.0f;

  r = physic::walk(g_character.pos, delta_time, self_hspeed, self_vspeed,
                   self_hforce, self_hspeed_min, self_hspeed_max, 800, true,
                   self_fh, {0, 0, 0, 0}, foothold_game_instance::data);
  return r;
}

bool character_logic_system::run_fall(game_character &g_character) {
  if (!g_character.abnormals.contains(
          game_character::abnormal_state_type::dizz)) {
    if (character_action_input.contains("left")) {
      self_hspeed -= 0.03 * window::delta_time;
    } else if (character_action_input.contains("right")) {
      self_hspeed += 0.03 * window::delta_time;
    }
  }
  auto vspeed = self_vspeed + window::delta_time * 2;
  if (self_vspeed <= 0 && vspeed > 0) {
    self_fall_min = g_character.pos.y;
  }
  self_vspeed = vspeed;
}

bool character_logic_system::run_prone(game_character &g_character) {
  if (g_character.abnormals.contains(
          game_character::abnormal_state_type::dizz)) {
    return false;
  }
  if (self_fh == 0 || self_lr != 0) {
    return false;
  }
  if (character_action_input.contains("down")) {
    run_action(g_character, u"prone");
    return true;
  }
  return false;
}

bool character_logic_system::run_jump(game_character &g_character) {
  if (g_character.abnormals.contains(
          game_character::abnormal_state_type::dizz)) {
    return false;
  }
  if (character_action_input.contains("jump")) {
    auto g_action = load_action_type(g_character);
    switch (g_action) {
    case action_enum::stand:
    case action_enum::alert:
    case action_enum::walk: {
      self_vspeed = self_vspeed_min * 0.1;
      self_fh = 0;
      self_lr = 0;
      break;
    }
    case action_enum::prone: {
      // 下跳
      const auto &c_fh = foothold_game_instance::data.at(self_fh);

      break;
    }
    case action_enum::jump: {
      break;
    }
    }
    run_action(g_character, u"jump");
    return true;
  }
}

bool character_logic_system::run_climb(game_character &g_character) {
  if (g_character.abnormals.contains(
          game_character::abnormal_state_type::dizz)) {
    return false;
  }
  if (self_ladderrope_cooldown > window::dt_now) {
    return false;
  }
  if (character_action_input.contains("up") ||
      character_action_input.contains("down")) {
    auto &pos = g_character.pos;
    for (const auto &lr : ladderrope_game_instance::data | std::views::values) {
      // 判断x坐标是否在梯子范围内
      if (pos.x >= lr.x - 15 && pos.x <= lr.x + 15) {
        if (character_action_input.contains("up")) {
          // 按上键

        } else {
          // 按下键
        }
      }
    }
  }
}

bool character_logic_system::run_climbing(game_character &g_character) {
  if (g_character.abnormals.contains(
          game_character::abnormal_state_type::dizz)) {
    run_action(g_character, u"jump");
    return false;
  }
  if (self_lr != 0) {
    return false;
  }
  if (character_action_input.contains("up")) {
    self_vspeed = -100;
  } else if (character_action_input.contains("down")) {
    self_vspeed = 100;
  } else {
    self_vspeed = 0;
  }
  g_character.action_animate = !(self_vspeed == 0);
  g_character.pos.y += self_vspeed * window::delta_time / 1000.0f;
  auto c_lr = ladderrope_game_instance::data.at(self_lr);
  if (g_character.pos.y < c_lr.t) {
    if (c_lr.uf == 1) {
      g_character.action_animate = true;
      g_character.pos.y = c_lr.t - 5;
      self_ladderrope_cooldown = window::dt_now + 80;
      self_vspeed = 0;
      run_action(g_character, u"jump");
    } else {
      g_character.action_animate = false;
    }
  } else if (g_character.pos.y > c_lr.b) {
    g_character.pos.y = c_lr.b;
    g_character.action_animate = true;
    self_ladderrope_cooldown = window::dt_now + 80;
    self_vspeed = 0;
    run_action(g_character, u"jump");
  }

  return true;
}

character_logic_system::pos_type
character_logic_system::load_pos_type(game_character &g_character) {
  auto swim_r =
      map_info_game_instance::load_swim_rect(scene_system_instance::map_id);
  if (swim_r.has_value()) {
    if (SDL_PointInRectFloat(&g_character.pos, &swim_r.value())) {
      return pos_type::swim;
    }
  } else {
    return pos_type::land;
  }
  return pos_type::land;
}

void character_logic_system::run_network_sync(game_character &g_character,
                                              game_character &o_character) {
  // 网络同步
  if (o_character.pos.x != g_character.pos.x ||
      o_character.pos.y != g_character.pos.y) {
    fbs::ClientCharacterMoveT client_character_move;
    fbs::MovementT m;
    m.x1 = o_character.pos.x;
    m.y1 = o_character.pos.y;

    m.x2 = g_character.pos.x;
    m.y2 = g_character.pos.y;

    m.action =
        std::string{g_character.action.begin(), g_character.action.end()};
    m.page = g_character.page;
    m.time = window::delta_time;
    m.flip = g_character.flip;

    client_character_move.movement =
        std::make_unique<fbs::MovementT>(std::move(m));

    client_request::client_character_move_request(client_character_move);
  }
}

character_logic_system::action_enum
character_logic_system::load_action_type(game_character &g_character) {
  const static std::flat_map<std::u16string, action_enum> map_name = {
      {u"stand1", action_enum::stand}, {u"stand2", action_enum::stand},
      {u"alert", action_enum::alert},  {u"walk1", action_enum::walk},
      {u"walk2", action_enum::walk},   {u"prone", action_enum::prone},
      {u"jump", action_enum::jump},
  };
  return map_name.at(g_character.action);
};

void character_logic_system::run_state_machine(game_character &g_character) {
  auto o_character = g_character;
  auto g_action = load_action_type(g_character);
  switch (g_action) {
  case action_enum::stand:
  case action_enum::alert:
  case action_enum::walk: {
    run_flip(g_character);
    run_pick(g_character);
    if (run_prone(g_character)) {
      break;
    }
    if (!run_walk(g_character)) {
      break;
    }
    if (run_jump(g_character)) {
    }
    break;
  }
  case action_enum::prone: {
    if (!run_prone(g_character)) {
      run_stand_action(g_character);
    }
    break;
  }
  case action_enum::jump: {
    run_flip(g_character);

    break;
  }
  default: {
    std::abort();
  }
  }
  run_network_sync(g_character, o_character);
}

void character_logic_system::run_others_movement() {
  for (auto &c : character_game_instance::others | std::views::values) {
    if (!c.movements.empty()) {
      auto &mv = c.movements[0];
      auto per = window::delta_time / (float)mv.time;
      // 自动处理大小顺序
      auto low = std::min(mv.x1, mv.x2);
      auto high = std::max(mv.x1, mv.x2);
      if (c.g_character.pos.x >= low && c.g_character.pos.x <= high) {
        per += std::abs((c.g_character.pos.x - mv.x1) / (high - low));
      } else {
        per = 0.0f;
      }
      per = std::min(per, 1.0f);
      auto per_x = mv.x1 + (mv.x2 - mv.x1) * per;
      auto per_y = mv.y1 + (mv.y2 - mv.y1) * per;
      c.g_character.pos.x = per_x;
      c.g_character.pos.y = per_y;
      if (per == 1.0f) {
        c.movements.erase(c.movements.begin());
      }
      c.g_character.flip = mv.flip;
    }
  }
}

void character_logic_system::run_others_animate() {
  for (auto &c : character_game_instance::others | std::views::values) {
    auto &character = c.g_character;
    run_animate(character);
  }
}

void character_logic_system::run_others() {
  run_others_movement();
  run_others_animate();
}

// 人物状态机
bool character_logic_system::run() {
  run_others();
  run_animate(character_game_instance::self);
  run_state_machine(character_game_instance::self);
  return true;
}