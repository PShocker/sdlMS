#include "character_logic_system.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_tomb.h"
#include "src/client/game_instance/afterimage_game_instance.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/drop_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/ladderrope_game_instance.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/portal_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/game_instance/reactor_game_instance.h"
#include "src/client/game_instance/seat_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/game_instance/triangle_game_instance.h"
#include "src/client/system/logic/mob_logic_system.h"
#include "src/client/system/ui/package_ui_system.h"
#include "src/client/system/ui/revive_ui_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/physic/physic.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_ball_instance.h"
#include "src/server/server_instance/server_character_instance.h"
#include "wz/Property.h"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <flat_map>
#include <flat_set>
#include <format>
#include <memory>
#include <numbers>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

static const std::flat_set<equip_game_instance::weapon_type> shoot_weapons = {
    equip_game_instance::weapon_type::BOW,
    equip_game_instance::weapon_type::CROSSBOW,
    equip_game_instance::weapon_type::CLAW,
    equip_game_instance::weapon_type::GUN,
};
static const std::flat_map<equip_game_instance::weapon_type,
                           std::flat_set<std::u16string>>
    weapon_attack_action = {
        {equip_game_instance::weapon_type::S1A1M1D,
         {u"stabO1", u"stabO2", u"swingO1", u"swingO2", u"swingO3"}},
        {equip_game_instance::weapon_type::SPEAR, {u"stabT1", u"swingP1"}},
        {equip_game_instance::weapon_type::BOW, {u"shoot1"}},
        {equip_game_instance::weapon_type::CROSSBOW, {u"shoot1"}},
        {equip_game_instance::weapon_type::S2A2M2,
         {u"stabO1", u"stabO2", u"swingT1", u"swingT2", u"swingT3"}},
        {equip_game_instance::weapon_type::WAND, {u"swingO1", u"swingO2"}},
        {equip_game_instance::weapon_type::CLAW, {u"swingO1", u"swingO2"}},
        {equip_game_instance::weapon_type::GUN, {u"shot"}},
};
static const std::flat_map<equip_game_instance::weapon_type,
                           std::flat_set<std::u16string>>
    weapon_attack_action2 = {
        {equip_game_instance::weapon_type::BOW, {u"swingT1", u"swingT3"}},
        {equip_game_instance::weapon_type::CROSSBOW, {u"swingT1", u"stabT1"}},
        {equip_game_instance::weapon_type::CLAW, {u"stabO1", u"stabO2"}},
        {equip_game_instance::weapon_type::GUN, {u"swingP1", u"stabT2"}},
};

SDL_FRect character_logic_system::load_rect(game_character &g_character) {
  const auto stand_w = 44.0f;
  const auto stand_h = 65.0f;
  const auto prone_w = 46.0f;
  const auto prone_h = 31.0f;
  SDL_FRect rect;
  if (g_character.action == u"prone" || g_character.action == u"proneStab") {
    rect.x = g_character.pos.x - prone_w / 2;
    rect.y = g_character.pos.y - prone_h;
    rect.w = prone_w;
    rect.h = prone_h;
  } else {
    rect.x = g_character.pos.x - stand_w / 2;
    rect.y = g_character.pos.y - stand_h;
    rect.w = stand_w;
    rect.h = stand_h;
  }
  if (g_character.flip == 1) {
    rect.x += 2 * (g_character.pos.x - rect.x) - rect.w;
  }
  return rect;
}

check_reactors character_logic_system::run_reactor_check(game_character &g,
                                                         SDL_FRect g_r) {
  auto &g_pos = g.pos;
  std::vector<check_reactors::reactors> v;
  std::flat_map<uint32_t, check_reactors::reactors> m;
  for (const auto &rs : reactor_game_instance::data) {
    for (const auto &r : rs) {
      auto node = wz_resource::reactor->find(r.id + u".img");
      node = node->find(std::to_string(r.state) + "/event/0");
      if (node == nullptr) {
        continue;
      }
      auto type =
          static_cast<wz::Property<int> *>(node->get_child(u"type"))->get();
      if (type != 0) {
        continue;
      }
      SDL_FRect reactor_r;
      reactor_r.x = r.pos.x - 20;
      reactor_r.y = r.pos.y - 20;
      reactor_r.w = 40;
      reactor_r.h = 40;
      if (SDL_HasRectIntersectionFloat(&reactor_r, &g_r)) {
        auto &r_pos = r.pos;
        auto dis = (r_pos.x - g_pos.x) * (r_pos.x - g_pos.x) +
                   (r_pos.y - g_pos.y) * (r_pos.y - g_pos.y);
        SDL_FRect res;
        SDL_GetRectIntersectionFloat(&reactor_r, &g_r, &res);
        float attack_x = res.x + res.w / 2 - r_pos.x;
        float attack_y = res.y + res.h / 2 - r_pos.y;
        m[dis] = {
            .r = r,
            .x = attack_x,
            .y = attack_y,
        };
      }
    }
  }
  v.append_range(m.values());
  return {v};
}

check_mobs character_logic_system::run_attack_check(game_character &g_character,
                                                    game_triangle tri) {
  std::vector<check_mobs::mobs> v;
  std::flat_map<uint32_t, check_mobs::mobs> m;
  auto &g_pos = g_character.pos;
  for (const auto [k, v] : mob_game_instance::data) {
    auto &mob = v.mob;
    if (mob.hp <= 0) {
      continue;
    }
    auto mob_action = mob_logic_system::load_action_type(mob.action);
    if (mob_action == mob_logic_system::action_enum::revive ||
        mob_action == mob_logic_system::action_enum::die) {
      continue;
    }

    auto m_r = mob_logic_system::load_rect(mob).value();
    auto t = triangle_game_instance::load_tri(tri, g_character.flip,
                                              g_character.pos);
    if (triangle_game_instance::rect_ins_tri(m_r, t)) {
      auto &m_pos = mob.pos;
      auto dis = (m_pos.x - g_pos.x) * (m_pos.x - g_pos.x) +
                 (m_pos.y - g_pos.y) * (m_pos.y - g_pos.y);
      float attack_x = 0;
      float attack_y = 0;
      m[dis] = {
          .mob = v.mob,
          .x = attack_x,
          .y = attack_y,
      };
    }
  }
  v.append_range(m.values());
  return {v};
}

check_mobs character_logic_system::run_attack_check(game_character &g_character,
                                                    SDL_FRect g_r) {
  std::vector<check_mobs::mobs> v;
  std::flat_map<uint32_t, check_mobs::mobs> m;
  auto &g_pos = g_character.pos;
  for (const auto [k, v] : mob_game_instance::data) {
    auto &mob = v.mob;
    auto mob_action = mob_logic_system::load_action_type(mob.action);
    if (mob_action == mob_logic_system::action_enum::revive ||
        mob_action == mob_logic_system::action_enum::die) {
      continue;
    }
    if (mob.hp <= 0) {
      continue;
    }
    auto m_r = mob_logic_system::load_rect(mob).value();
    if (SDL_HasRectIntersectionFloat(&m_r, &g_r)) {
      auto &m_pos = mob.pos;
      auto dis = (m_pos.x - g_pos.x) * (m_pos.x - g_pos.x) +
                 (m_pos.y - g_pos.y) * (m_pos.y - g_pos.y);
      SDL_FRect res;
      SDL_GetRectIntersectionFloat(&m_r, &g_r, &res);
      float attack_x = res.x + res.w / 2 - m_pos.x;
      float attack_y = res.y + res.h / 2 - m_pos.y;
      m[dis] = {
          .mob = v.mob,
          .x = attack_x,
          .y = attack_y,
      };
    }
  }
  v.append_range(m.values());
  return {v};
}

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

bool character_logic_system::run_face_action(game_character &g_character,
                                             const std::u16string &action) {
  if (g_character.face.id == action) {
    return false;
  }
  g_character.face.action = action;
  g_character.face.index = 0;
  g_character.face.time = 0;
  g_character.face.destroy = window::dt_now + 4000;
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

void character_logic_system::run_climb_action(game_character &g_character) {
  const auto &c_lr = ladderrope_game_instance::data.at(self_lr);
  if (c_lr.l == 1) {
    run_action(g_character, u"ladder");
  } else {
    run_action(g_character, u"rope");
  }
  return;
}

bool character_logic_system::run_animate(game_character &g_character) {
  bool r = false;
  if (!g_character.action_animate) {
    return r;
  }
  auto delta = window::delta_time;
  auto action_type = load_action_type(g_character);
  float speed = 1;
  switch (action_type) {
  case action_enum::attack:
  case action_enum::skill: {
    speed = load_attack_speed(g_character);
    break;
  }
  default: {
    break;
  }
  }
  g_character.action_time += delta * speed;
  //   获取动作延迟时间
  uint32_t delay = 100;
  uint32_t size = 0;
  if (!g_character.morph.empty()) {
    auto node = wz_resource::morph->find(g_character.morph + u".img");
    auto action = load_morph_type(g_character);
    node = node->get_child(action);
    size = node->children_count();
    node = node->get_child(std::to_string(g_character.action_index));
    if (node->get_child(u"delay")) {
      delay =
          static_cast<wz::Property<int> *>(node->get_child(u"delay"))->get();
    }
  } else if (character_game_instance::extern_action.contains(
                 g_character.action)) {
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
    r = g_character.action_index >= size;
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
  if (self_flip_cooldown > window::dt_now) {
    return false;
  }
  if (character_action_input.contains("left")) {
    g_character.flip = 0;
    self_flip_cooldown = window::dt_now + 50;
    return true;
  }
  if (character_action_input.contains("right")) {
    g_character.flip = 1;
    self_flip_cooldown = window::dt_now + 50;
    return true;
  }
  return false;
}

void character_logic_system::run_pick(game_character &g_character) {
  if (ccp.random_id != 0) {
    return;
  }
  if (g_character.abnormals.contains(
          game_character::abnormal_state_type::dizz)) {
    return;
  }
  if (character_action_input.contains("pick")) {
    for (auto &v : drop_game_instance::data | std::views::values) {
      if (v.type == game_drop::drop_enum::land) {
        if (package_game_instance::load_empty_index(v.data).empty()) {
          continue;
        }
        auto pos = g_character.pos;
        if (pos.x == std::clamp(pos.x, v.goal.x - 20, v.goal.x + 40) &&
            pos.y == std::clamp(pos.y, v.goal.y - 20, v.goal.y + 20)) {
          ccp.map_id = scene_system_instance::map_id;
          ccp.random_id = v.random_id;
          client_request::send_to_host(ccp);
          return;
        }
      }
    }
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

  auto map_id = scene_system_instance::map_id;
  auto border = map_info_game_instance::load_mr_border(map_id);

  r = physic::walk(g_character.pos, delta_time, self_hspeed, self_vspeed,
                   self_hforce, self_hspeed_min, self_hspeed_max, 800, true,
                   self_fh, border, foothold_game_instance::data);
  return r;
}

bool character_logic_system::run_fall(game_character &g_character) {
  if (self_fh != 0) {
    return false;
  }
  if (!g_character.abnormals.contains(
          game_character::abnormal_state_type::dizz)) {
    if (character_action_input.contains("left")) {
      self_hspeed -= 0.1 * window::delta_time;
    } else if (character_action_input.contains("right")) {
      self_hspeed += 0.1 * window::delta_time;
    }
  }
  auto delta_time = window::delta_time / 1000.0;
  auto vspeed = self_vspeed + delta_time * 2000;
  if (self_vspeed <= 0 && vspeed > 0) {
    self_fall_min = g_character.pos.y;
  }
  self_vspeed = vspeed;
  auto fall_collide = self_foothold_cooldown <= window::dt_now;

  auto map_id = scene_system_instance::map_id;
  auto border = map_info_game_instance::load_mr_border(map_id);

  bool r =
      physic::fall(g_character.pos, delta_time, self_hspeed, self_vspeed,
                   self_vspeed_min, self_vspeed_max, border, fall_collide, true,
                   self_fh, g_character.page, foothold_game_instance::data);
  return r;
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
    self_hspeed = 0;
    self_vspeed = 0;
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
      break;
    }
    case action_enum::prone: {
      // 下跳
      const auto &c_fh = foothold_game_instance::data.at(self_fh);
      if (!c_fh.forbidFallDown) {
        for (auto &fh : foothold_game_instance::data | std::views::values) {
          if (fh.x1 > fh.x2) {
            continue;
          }
          const auto &pos = g_character.pos;
          if (pos.x < fh.x1 || pos.x > fh.x2) {
            continue;
          }
          if (fh.k.has_value()) {
            auto y = fh.k.value() * pos.x + fh.intercept.value();
            if (y >= pos.y + 10 && y <= pos.y + 600) {
              self_vspeed = -100;
              self_hspeed = 0;
              self_fh = 0;
              self_foothold_cooldown = window::dt_now + 120;
              run_action(g_character, u"jump");
              return true;
            }
          }
        }
      }
      return false;
      break;
    }
    case action_enum::jump: {
      // double jump
    }
    case action_enum::climb: {
      if (character_action_input.contains("up") ||
          character_action_input.contains("down")) {
        return false;
        break;
      }
      if (character_action_input.contains("left") ||
          character_action_input.contains("right")) {
        self_hspeed = character_action_input.contains("left") ? -140 : 140;
        g_character.action_animate = true;
        self_vspeed = -310;
        self_lr = 0;
        self_ladderrope_cooldown = window::dt_now + 120;
      } else {
        return false;
      }
      break;
    }
    default: {
      break;
    }
    }
    self_fh = 0;
    run_action(g_character, u"jump");
    audio_game_instance::load_audio(u"Game.img/Jump", 0);
    return true;
  }
  return false;
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
    for (const auto &[id, lr] : ladderrope_game_instance::data) {
      // 判断x坐标是否在梯子范围内
      if (pos.x >= lr.x - 15 && pos.x <= lr.x + 15) {
        float y_min = lr.t;
        float y_max = lr.b + 5;
        if (character_action_input.contains("up")) {
          // 按上键
          if (self_fh != 0) {
            // 可爬范围为[b,b+5]
            y_min = lr.b;
            y_max = lr.b + 5;
          }
        } else {
          // 按下键
          if (self_fh != 0) {
            y_min = lr.t - 10;
            y_max = lr.t;
          }
        }

        if (pos.y >= y_min && pos.y <= y_max) {
          self_lr = id;
          self_vspeed = 0;
          self_hspeed = 0;
          pos.x = lr.x;
          pos.y = std::clamp(pos.y, (float)lr.t, (float)lr.b);
          g_character.page = lr.page;
          run_climb_action(g_character);
          return true;
        }
      }
    }
  }
  return false;
}

bool character_logic_system::run_climbing(game_character &g_character) {
  if (g_character.abnormals.contains(
          game_character::abnormal_state_type::dizz)) {
    run_action(g_character, u"jump");
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
      self_fh = 0;
      self_lr = 0;
      return false;
    } else {
      g_character.action_animate = false;
      g_character.pos.y = c_lr.t;
    }
  } else if (g_character.pos.y > c_lr.b) {
    g_character.pos.y = c_lr.b;
    g_character.action_animate = true;
    self_ladderrope_cooldown = window::dt_now + 80;
    self_vspeed = 0;
    self_fh = 0;
    self_lr = 0;
    return false;
  }

  return true;
}

bool character_logic_system::run_sit(game_character &g_character) {
  if (!g_character.morph.empty()) {
    return false;
  }
  if (character_action_input.contains("sit")) {
    if (self_sit_cooldown >= window::dt_now) {
      for (const auto &[seat_pos] : seat_game_instance::data) {
        if (std::abs(g_character.pos.x - seat_pos.x) <= 20 &&
            std::abs(g_character.pos.y - seat_pos.y) <= 20) {
          run_action(g_character, u"sit");
          self_sit_cooldown = window::dt_now + 120;
          return true;
        }
      }
    }
  }
  return false;
}

bool character_logic_system::run_sitting(game_character &g_character) {
  if (!character_action_input.empty()) {
    if (self_sit_cooldown <= window::dt_now) {
      run_unsit_chair(g_character);
      return false;
    }
  }
  if (self_alert_cooldown > window::dt_now) {
    run_action(g_character, u"alert");
    return false;
  }
  return true;
}

bool character_logic_system::run_skill(game_character &g_character,
                                       const std::u16string &id) {
  if (g_character.abnormals.contains(
          game_character::abnormal_state_type::dizz)) {
    return false;
  }
  if (!g_character.morph.empty()) {
    return false;
  }
  auto &skis = skill_game_instance::skis();
  if (skis.contains(id)) {
    auto ski_lv = job_skill_game_instance::load_ski_level(id);
    const auto &ski = skis[id];
    if (!ski.use) {
      return false;
    }
    if (ski.cd < window::dt_now) {
      auto action_type = load_action_type(g_character);
      switch (action_type) {
      case action_enum::jump: {
        if (ski.fall) {
          ski.use(ski_lv);
        }
        break;
      }
      case action_enum::climb: {
        if (ski.climb) {
          ski.use(ski_lv);
        }
        break;
      }
      default: {
        ski.use(ski_lv);
        break;
      }
      }
    }
  }
  return true;
}

bool character_logic_system::run_skill(game_character &g_character) {
  if (!character_skill_input.empty()) {
    const auto &id = *character_skill_input.begin();
    return run_skill(g_character, {id.begin(), id.end()});
  }
  return false;
}

bool character_logic_system::run_attack_action(game_character &g_character) {
  if (g_character.abnormals.contains(
          game_character::abnormal_state_type::dizz)) {
    return false;
  }
  if (!g_character.weapon.has_value()) {
    return false;
  }
  auto g_action = load_action_type(g_character);
  auto g_weapon = g_character.weapon->id;
  auto g_weapon_info = equip_game_instance::load_equip_info(g_weapon);
  auto weapon_type = equip_game_instance::load_weapon_type(g_character);
  bool shoot = true;
  switch (g_action) {
  case action_enum::stand:
  case action_enum::alert:
  case action_enum::walk: {
    self_hspeed = 0;
  }
  case action_enum::jump: {
    auto &gen = random_game_instance::gen;
    auto actions = &weapon_attack_action.at(weapon_type);
    std::uniform_int_distribution<> dis(0, actions->size() - 1);
    auto selected = *std::next(actions->begin(), dis(gen));
    run_action(g_character, selected);
    break;
  }
  case action_enum::prone: {
    run_action(g_character, u"proneStab");
    break;
  }
  default: {
    break;
  }
  }
  return true;
}

bool character_logic_system::run_attack(game_character &g_character) {
  if (g_character.abnormals.contains(
          game_character::abnormal_state_type::dizz)) {
    return false;
  }
  if (!g_character.morph.empty()) {
    return false;
  }
  if (!g_character.weapon.has_value()) {
    return false;
  }
  if (self_attack_cooldown > window::dt_now) {
    return false;
  }
  auto g_action = load_action_type(g_character);
  if (g_action == action_enum::attack || g_action == action_enum::skill) {
    return false;
  }
  if (character_action_input.contains("attack")) {
    auto g_weapon = g_character.weapon->id;
    auto g_weapon_info = equip_game_instance::load_equip_info(g_weapon);
    uint64_t delay;
    auto weapon_type = equip_game_instance::load_weapon_type(g_character);
    bool shoot_weapon = shoot_weapons.contains(weapon_type);
    auto &gen = random_game_instance::gen;
    const std::flat_set<std::u16string> *actions;
    if (weapon_attack_action2.contains(weapon_type)) {
      actions = &weapon_attack_action2.at(weapon_type);
    } else {
      actions = &weapon_attack_action.at(weapon_type);
    }
    std::uniform_int_distribution<> dis(0, actions->size() - 1);
    auto selected = *std::next(actions->begin(), dis(gen));
    run_action(g_character, selected);
    SDL_FRect g_r = afterimage_game_instance::load_rect(g_character).value();
    auto rt = run_reactor_check(g_character, g_r);
    switch (g_action) {
    case action_enum::stand:
    case action_enum::alert:
    case action_enum::walk: {
      self_hspeed = 0;
    }
    case action_enum::jump: {
      if (g_action == action_enum::jump &&
          (weapon_type == equip_game_instance::weapon_type::BOW ||
           weapon_type == equip_game_instance::weapon_type::CROSSBOW)) {
        shoot_weapon = false;
      }
      if (!rt.data.empty()) {
        shoot_weapon = false;
      }
      if (!package_game_instance::load_active_ball()) {
        shoot_weapon = false;
      }
      actions = &weapon_attack_action.at(weapon_type);
      if (!shoot_weapon) {
        if (weapon_attack_action2.contains(weapon_type)) {
          actions = &weapon_attack_action2.at(weapon_type);
        }
      }
      std::uniform_int_distribution<> dis(0, actions->size() - 1);
      auto selected = *std::next(actions->begin(), dis(gen));
      run_action(g_character, selected);
      break;
    }
    case action_enum::prone: {
      run_action(g_character, u"proneStab");
      break;
    }
    default: {
      return false;
      break;
    }
    }
    delay = afterimage_game_instance::load_beat_time(g_character);
    self_alert_cooldown = window::dt_now + 5000;
    load_sfx(g_character);
    // reactor
    if (!rt.data.empty()) {
      auto &r = rt.data[0];
      ClientReactorT crt;
      crt.map_id = scene_system_instance::map_id;
      crt.payload = std::make_unique<ReactorT>();
      crt.payload->reactor_index = r.r.index;
      crt.payload->delay = delay;
      client_request::send_to_host(crt);
      return true;
    }
    if (shoot_weapon) {
      game_triangle tri = {
          {
              SDL_FPoint{-350, -100},
              SDL_FPoint{-350, 100},
              SDL_FPoint{0, -28},
          },
      };
      auto cm = character_logic_system::run_attack_check(g_character, tri);
      auto &ball = *package_game_instance::load_active_ball();
      auto &consume = static_cast<game_consume_item &>(*ball);
      consume.num -= 1;
      auto cash_ball = package_game_instance::load_active_cash_ball();
      std::u16string path;
      std::u16string effect;
      if (!cash_ball.empty()) {
        auto ball_sub_id = cash_ball.substr(0, 4) + u".img";
        path = u"Cash/" + ball_sub_id + u"/" + cash_ball + u"/bullet";
        effect = u"Cash/" + ball_sub_id + u"/" + cash_ball + u"/hit";

      } else {
        auto ball_id = ball->id;
        auto ball_sub_id = ball_id.substr(0, 4) + u".img";
        path = u"Consume/" + ball_sub_id + u"/" + ball_id + u"/bullet";
        effect = u"Afterimage/hit.img/maceF";
      }
      auto page = g_character.page;
      SDL_FPoint pos = g_character.pos;
      pos.y -= 28;
      SDL_FPoint goal = pos;
      if (g_character.flip) {
        goal.x += 350;
      } else {
        goal.x -= 350;
      }
      auto cct = ball_game_instance::create_ball_payload(cm, pos, goal, delay,
                                                         page, 700, path);
      if (!cm.data.empty()) {
        cm.data = {cm.data[0]};
        cm.data[0].hits = {100};
        auto d = ball_game_instance::load_ball_time(cct);
        auto cat = skill_game_instance::create_attack_payload(cm, pos, d);
        cat.payload[0]->effect = {effect.begin(), effect.end()};
        client_request::send_to_host(cat);
      }
      server_ball_instance::handle_server_b(cct.payload);
      client_request::send_to_host(cct);
    } else {
      auto cm = run_attack_check(g_character, g_r);
      if (!cm.data.empty()) {
        cm.data = {cm.data[0]};
        cm.data[0].hits = {30};
        auto cat = skill_game_instance::create_attack_payload(
            cm, g_character.pos, delay);
        auto hit_type = afterimage_game_instance::load_hit_type(g_character);
        cat.payload[0]->effect = "Afterimage/hit.img/" +
                                 std::string{hit_type.begin(), hit_type.end()};
        client_request::send_to_host(cat);
      }
    }

    return true;
  }
  return false;
}

bool character_logic_system::run_portal(game_character &g_character) {
  // 特殊情况，如果掉到了地图外,直接传送到sp出生点
  auto border =
      map_info_game_instance::load_mr_border(scene_system_instance::map_id);
  if (g_character.pos.y >= border.h) {
    // no change map
    const auto &tn = portal_game_instance::data.find(u"sp")->second;
    g_character.pos = tn.pos;
    g_character.pos.y -= 5;
    self_hspeed = 0;
    self_vspeed = 0;
    self_fh = 0;
    run_action(g_character, u"jump");
    self_portal_cooldown = window::dt_now + 1200;
    return true;
  }
  if (g_character.abnormals.contains(
          game_character::abnormal_state_type::dizz)) {
    return false;
  }
  if (self_portal_cooldown <= window::dt_now) {
    bool up = character_action_input.contains("up");
    for (const auto &por : portal_game_instance::data | std::views::values) {
      if (por.tm == 999999999) {
        continue;
      }
      if (((por.pt == 1 || por.pt == 2) && up) || por.pt == 3) {
        const auto &g_pos = g_character.pos;
        const auto &p_pos = por.pos;
        if (g_pos.x == std::clamp(g_pos.x, p_pos.x - 40, p_pos.x + 40) &&
            g_pos.y == std::clamp(g_pos.y, p_pos.y - 50, p_pos.y + 50)) {
          if (por.tm != scene_system_instance::map_id) {
            // need to change map
            scene_system_instance::enter_prepare(por.tm, por.tn, 0);
            self_portal_cooldown = window::dt_now + 1000;
            audio_game_instance::load_audio(u"Game.img/Portal", 0);
          } else {
            // no change map
            const auto &tn = portal_game_instance::data.find(por.tn)->second;
            g_character.pos = tn.pos;
            g_character.pos.y -= 5;
            self_hspeed = 0;
            self_vspeed = 0;
            self_fh = 0;
            run_action(g_character, u"jump");
            self_portal_cooldown = window::dt_now + 1200;
            audio_game_instance::load_audio(u"Game.img/Portal", 0);
            return true;
          }
        }
      }
    }
  }
  return false;
}

bool character_logic_system::run_face(game_character &g_character) {
  if (!character_face_input.empty()) {
    auto &face = *character_face_input.begin();
    run_face_action(g_character, std::u16string{face.begin(), face.end()});
    return true;
  }
  return false;
}

void character_logic_system::run_face_animate(game_character &g_character) {
  g_character.face.time += window::delta_time;
  const auto &delays =
      character_game_instance::face_data.at(g_character.face.id)
          .delay.at(g_character.face.action);
  auto delay = delays[g_character.face.index];
  if (delay != 0) {
    if (g_character.face.time >= delay) {
      g_character.face.index += 1;
      if (g_character.face.index >= delays.size()) {
        if (g_character.face.action == u"blink" ||
            (g_character.face.destroy <= window::dt_now)) {
          g_character.face.action = u"default";
          g_character.face.destroy = window::dt_now + 4000;
        }
        g_character.face.index = 0;
      }
      g_character.face.time = 0;
    }
    return;
  }
  if (g_character.face.destroy <= window::dt_now) {
    if (g_character.face.action == u"default") {
      g_character.face.action = u"blink";
    } else {
      g_character.face.action = u"default";
    }
    g_character.face.index = 0;
    g_character.face.time = 0;
    g_character.face.destroy = window::dt_now + 4000;
  }
  return;
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

void character_logic_system::run_network_sync_state() {
  static int32_t hp;
  static int32_t max_hp;
  if (hp != character_stat_game_instance::hp_point) {
    StateT st;
    st.state = StateEnum_HP;
    st.val = character_stat_game_instance::hp_point;
    ccs.payload.push_back(std::make_unique<StateT>(st));
    hp = character_stat_game_instance::hp_point;
  }
  if (max_hp != character_stat_game_instance::hp_point_max) {
    StateT st;
    st.state = StateEnum_MAX_HP;
    st.val = character_stat_game_instance::hp_point_max;
    ccs.payload.push_back(std::make_unique<StateT>(st));
    max_hp = character_stat_game_instance::hp_point_max;
  }
  if (!ccs.payload.empty()) {
    ccs.map_id = scene_system_instance::map_id;
    client_request::send_to_host(ccs);
  }
  ccs.payload.clear();
}

void character_logic_system::run_network_sync() {
  static uint64_t time = 0;
  const int32_t MIN_SEND_INTERVAL_MS = 33;
  // 节流：频率限制
  if (window::dt_now - time <= MIN_SEND_INTERVAL_MS) {
    return;
  }
  time = window::dt_now;
  const auto &g_character = character_game_instance::self;
  auto map_id = scene_system_instance::map_id;

  static SDL_FPoint pos;
  if ((g_character.pos.x != pos.x || g_character.pos.y != pos.y) &&
      g_character.action != u"dead") {
    MovementT mv;
    mv.x1 = pos.x;
    mv.y1 = pos.y;
    mv.x2 = g_character.pos.x;
    mv.y2 = g_character.pos.y;
    mv.page = g_character.page;
    mv.time = std::min(window::delta_time, MIN_SEND_INTERVAL_MS);
    pos = g_character.pos;
    ClientCharacterMvT mvt;
    mvt.map_id = map_id;
    mvt.payload = std::make_unique<MovementT>(mv);
    client_request::send_to_host(mvt);
  }
  static bool flip;
  if (flip != g_character.flip) {
    FlipT f;
    f.flip = g_character.flip;
    ClientCharacterFlipT fpt;
    fpt.map_id = map_id;
    fpt.payload = std::make_unique<FlipT>(f);
    client_request::send_to_host(fpt);
    flip = g_character.flip;
  }
  static std::u16string action;
  static bool action_animate;
  if (action != g_character.action ||
      action_animate != g_character.action_animate) {
    ActionT a;
    a.action = {g_character.action.begin(), g_character.action.end()};
    a.action_animate = g_character.action_animate;
    a.action_index = g_character.action_index;

    ClientCharacterActionT cct;
    cct.map_id = map_id;
    cct.payload = std::make_unique<ActionT>(a);

    client_request::send_to_host(cct);
    action = g_character.action;
    action_animate = g_character.action_animate;
  }

  static std::u16string face;
  if (face != g_character.face.action) {
    if (g_character.face.action != u"blink") {
      FaceT ft;
      ft.face_action = {
          g_character.face.action.begin(),
          g_character.face.action.end(),
      };
      ClientCharacterFcT fct;
      fct.map_id = map_id;
      fct.payload = std::make_unique<FaceT>(ft);
      client_request::send_to_host(fct);
      face = g_character.face.action;
    }
  }

  if (cct.map_id != 0) {
    auto c = server_character_instance::load_charactert(g_character);
    cct.payload = std::make_unique<fbs::CharacterT>(std::move(c));
    client_request::send_to_host(cct);
    cct.map_id = 0;
  }

  run_network_sync_state();
}

void character_logic_system::run_network_die_sync(game_character &g_character) {
  DieT d;
  d.x = g_character.pos.x;
  d.y = g_character.pos.y;
  ClientCharacterDieT req;
  req.map_id = scene_system_instance::map_id;
  req.payload = std::make_unique<DieT>(d);
  client_request::send_to_host(req);
  return;
}

character_logic_system::action_enum
character_logic_system::load_action_type(game_character &g_character) {
  const static std::flat_map<std::u16string, action_enum> map_name = {
      {u"stand1", action_enum::stand},    {u"stand2", action_enum::stand},
      {u"alert", action_enum::alert},     {u"walk1", action_enum::walk},
      {u"walk2", action_enum::walk},      {u"prone", action_enum::prone},
      {u"jump", action_enum::jump},       {u"ladder", action_enum::climb},
      {u"rope", action_enum::climb},      {u"proneStab", action_enum::attack},
      {u"stabO1", action_enum::attack},   {u"stabO2", action_enum::attack},
      {u"swingO1", action_enum::attack},  {u"swingO2", action_enum::attack},
      {u"swingO3", action_enum::attack},  {u"stabT1", action_enum::attack},
      {u"swingP1", action_enum::attack},  {u"shoot1", action_enum::attack},
      {u"swingT1", action_enum::attack},  {u"swingT2", action_enum::attack},
      {u"swingT3", action_enum::attack},  {u"shot", action_enum::attack},
      {u"stabT2", action_enum::attack},   {u"dead", action_enum::dead},
      {u"alert2", action_enum::skill},    {u"alert3", action_enum::skill},
      {u"alert4", action_enum::skill},    {u"alert5", action_enum::skill},
      {u"assaulter", action_enum::skill}, {u"avenger", action_enum::skill},
      {u"burster1", action_enum::skill},  {u"burster2", action_enum::skill},
      {u"craft", action_enum::skill},     {u"crawl", action_enum::skill},
      {u"ladder2", action_enum::skill},   {u"magic1", action_enum::skill},
      {u"magic2", action_enum::skill},    {u"magic3", action_enum::skill},
      {u"prone2", action_enum::skill},    {u"rope2", action_enum::skill},
      {u"savage", action_enum::skill},    {u"shoot6", action_enum::skill},
      {u"shootDb1", action_enum::skill},  {u"shotC1", action_enum::skill},
      {u"sit", action_enum::sit},
  };
  auto action = map_name.at(g_character.action);
  if (action == action_enum::dead) {
    return action;
  }
  if (g_character.skill.has_value()) {
    if (action != action_enum::skill) {
      return action;
    }
    return action_enum::skill;
  }
  return action;
};

void character_logic_system::run_tomb(game_character &g_character) {
  if (!g_character.tomb.has_value()) {
    return;
  }
  auto &tomb = g_character.tomb.value();
  tomb.pos.y += window::delta_time * 0.4;
  tomb.pos.y = std::min(tomb.b.y, tomb.pos.y);
  tomb.ani_time += window::delta_time;
  auto node = wz_resource::effect->find(u"Tomb.img/" + tomb.ani_type);
  auto texture_node = node->get_child(std::to_string(tomb.ani_index));
  if (texture_node->type == wz::Type::UOL) {
    texture_node =
        static_cast<wz::Property<wz::WzUOL> *>(texture_node)->get_uol();
  }
  auto delay =
      static_cast<wz::Property<int> *>(texture_node->get_child(u"delay"))
          ->get();
  if (tomb.ani_time >= delay) {
    tomb.ani_time = 0;
    tomb.ani_index += 1;
    if (tomb.ani_index >= node->children_count()) {
      if (tomb.ani_type == u"fall") {
        tomb.ani_type = u"land";
      }
      tomb.ani_index = 0;
    }
  }

  const float velocity = 0.05f; // 角速度
  const float radius = 10.0f;   // 半径
  g_character.pos.x = tomb.b.x + radius * std::cos(tomb.rotation);
  g_character.pos.y = tomb.b.y - 10 + radius * std::sin(tomb.rotation);
  tomb.rotation += velocity;
  if (tomb.rotation >= 2 * std::numbers::pi) {
    tomb.rotation -= 2 * std::numbers::pi; // 保持角度在 [0, 2π) 范围内
  }
  return;
}

void character_logic_system::run_animate_chair(game_character &g_character) {
  if (!g_character.chair.has_value()) {
    return;
  }
  auto chair_id = g_character.chair->id;
}

void character_logic_system::run_state_machine() {
  auto &g_character = character_game_instance::self;
  auto o_character = g_character;
  run_item(g_character);
  auto g_action = load_action_type(g_character);
  run_face(g_character);
  run_face_animate(g_character);
  switch (g_action) {
  case action_enum::stand:
  case action_enum::alert:
  case action_enum::walk: {
    run_animate(g_character);
    run_flip(g_character);
    run_pick(g_character);
    if (run_climb(g_character)) {
      break;
    }
    if (run_prone(g_character)) {
      break;
    }
    if (!run_walk(g_character)) {
      run_action(g_character, u"jump");
      break;
    }
    if (run_attack(g_character)) {
      break;
    }
    if (run_skill(g_character)) {
      break;
    }
    if (run_portal(g_character)) {
      break;
    }
    run_jump(g_character);
    break;
  }
  case action_enum::prone: {
    run_flip(g_character);
    run_pick(g_character);
    if (!run_prone(g_character)) {
      run_stand_action(g_character);
    }
    if (run_jump(g_character)) {
      break;
    }
    if (run_climb(g_character)) {
      break;
    }
    if (run_attack(g_character)) {
      break;
    }
    if (run_portal(g_character)) {
      break;
    }
    break;
  }
  case action_enum::jump: {
    run_flip(g_character);
    if (!run_fall(g_character)) {
      // 刚落地后，瞬间动作不一定是stand，需要再进行一次状态机
      run_stand_action(g_character);
      run_state_machine();
      break;
    }
    if (run_attack(g_character)) {
      break;
    }
    if (run_skill(g_character)) {
      break;
    }
    if (run_climb(g_character)) {
      break;
    }
    if (run_portal(g_character)) {
      break;
    }
    break;
  }
  case action_enum::climb: {
    run_animate(g_character);
    run_flip(g_character);
    if (run_jump(g_character)) {
      break;
    }
    if (!run_climbing(g_character)) {
      run_action(g_character, u"jump");
    }
    if (run_portal(g_character)) {
      break;
    }
    if (run_skill(g_character)) {
      break;
    }
    break;
  }
  case action_enum::sit: {
    if (!run_sitting(g_character)) {
      run_state_machine();
    }
    break;
  }
  case action_enum::skill:
  case action_enum::attack: {
    bool fall = run_fall(g_character);
    if (run_animate(g_character)) {
      // 冷却
      self_flip_cooldown = window::dt_now + 33;
      self_attack_cooldown = window::dt_now + 33;
      if (!fall) {
        // 落地
        self_vspeed = 0;
        self_hspeed = 0;
        run_stand_action(g_character);
      } else {
        run_action(g_character, u"jump");
      }
      g_character.skill = std::nullopt;
      run_state_machine();
    }
    break;
  }
  case action_enum::dead: {
    run_tomb(g_character);
    break;
  }
  default: {
    std::abort();
  }
  }
  run_network_sync();
}

void character_logic_system::run_others_mv() {
  for (auto &c : character_game_instance::others | std::views::values) {
    auto &mvs = c.mvs;
    if (mvs.empty()) {
      continue;
    }
    auto mv = mvs[0];
    auto per = window::delta_time / (float)mv.time;
    // 计算当前点在线段上的进度（基于 x 和 y）
    float dx = mv.x2 - mv.x1;
    float dy = mv.y2 - mv.y1;
    float length_sq = dx * dx + dy * dy;

    if (length_sq > 0) {
      // 投影参数 t：当前点在线段上的位置（0=起点，1=终点）
      float t = ((c.g_character.pos.x - mv.x1) * dx +
                 (c.g_character.pos.y - mv.y1) * dy) /
                length_sq;
      t = std::clamp(t, 0.0f, 1.0f);
      per += t;
    }
    per = std::min(per, 1.0f);

    auto per_x = mv.x1 + (mv.x2 - mv.x1) * per;
    auto per_y = mv.y1 + (mv.y2 - mv.y1) * per;
    c.g_character.pos.x = per_x;
    c.g_character.pos.y = per_y;
    c.g_character.page = mv.page;
    if (per >= 1.0f) {
      mvs.erase(mvs.begin());
    }
  }
}

void character_logic_system::run_others_state_machine() {
  for (auto &c : character_game_instance::others | std::views::values) {
    auto &g_character = c.g_character;
    run_face_animate(g_character);
    auto g_action = load_action_type(g_character);
    switch (g_action) {
    case action_enum::stand:
    case action_enum::alert:
    case action_enum::walk:
    case action_enum::climb: {
      run_animate(g_character);
      break;
    }
    case action_enum::attack:
    case action_enum::skill: {
      auto action_index = g_character.action_index;
      auto action_time = g_character.action_time;
      if (run_animate(g_character)) {
        g_character.skill = std::nullopt;
        // 一次性动作,但是不知道后续是什么动作,保留动作
        g_character.action_index = action_index;
        g_character.action_time = action_time;
        g_character.action_animate = false;
      } else {
        g_character.action_animate = true;
      }
      break;
    }
    case action_enum::dead: {
      run_tomb(g_character);
      break;
    }
    case action_enum::sit: {
      run_animate_chair(g_character);
      break;
    }
    default: {
      break;
    }
    }
  }
}

void character_logic_system::run_others() {
  run_others_state_machine();
  run_others_mv();
}

void character_logic_system::run_color() {
  auto g_character = &character_game_instance::self;
  auto cooldown = character_logic_system::self_invincible_cooldown;
  if (cooldown >= window::dt_now) {
    auto delta = cooldown - window::dt_now;
    bool dark = (delta % 200) > 100;
    Uint8 color = dark ? 128 : 255;
    g_character->color = {color, color, color, 255};
  } else {
    g_character->color = {255, 255, 255, 255};
  }
}

// 人物状态机
bool character_logic_system::run() {
  run_others();
  run_state_machine();
  run_color();
  return true;
}

void character_logic_system::run_die_action(game_character &g_character) {
  run_action(g_character, u"dead");
  if (self_fh == 0 || self_lr == 0) {
    auto o_character = g_character;
    // 空中
    self_hspeed = 0;
    self_vspeed = self_vspeed_max;
    //
    auto map_id = scene_system_instance::map_id;
    auto border = map_info_game_instance::load_mr_border(map_id);

    float max_float = 10000;
    physic::fall(g_character.pos, max_float, self_hspeed, self_vspeed,
                 self_vspeed_min, self_vspeed_max, border, true, true, self_fh,
                 g_character.page, foothold_game_instance::data);
  }
  game_tomb t{
      .ani_type = u"fall",
      .pos = {g_character.pos.x, g_character.pos.y - 300},
      .b = g_character.pos,
  };
  g_character.tomb = t;

  self_invincible_cooldown = 0;
  self_vspeed = 0;
  self_hspeed = 0;
  self_alert_cooldown = 0;

  run_network_die_sync(g_character);

  audio_game_instance::load_audio(u"Game.img/Tombstone", 0);

  revive_ui_system::toggle();
}

void character_logic_system::load_sfx(game_character &g_character) {
  auto delay = afterimage_game_instance::load_beat_time(g_character);
  auto g_weapon = g_character.weapon->id;
  auto g_weapon_info = equip_game_instance::load_equip_info(g_weapon);
  auto sfx = static_cast<wz::Property<std::u16string> *>(
                 g_weapon_info->get_child(u"sfx"))
                 ->get() +
             u"/Attack";
  audio_game_instance::load_audio(u"Weapon.img/" + sfx, delay);
}

float character_logic_system::load_attack_speed(game_character &g_character) {
  if (!g_character.weapon.has_value()) {
    return 1.0;
  }
  auto w_speed = equip_game_instance::load_equip_inc(g_character.weapon->id)
                     .at(equip_game_instance::inc_type::WEAPON_SPEED);
  float speed = w_speed + g_character.attack_speed;
  return 1.6 - speed / 10;
}

void character_logic_system::run_sit_chair(game_character &g_character,
                                           const std::u16string &id) {
  run_action(g_character, u"sit");
  g_character.chair = game_chair{};
  g_character.chair->id = id;
  if (&g_character == &character_game_instance::self) {
    StateT st;
    st.state = fbs::StateEnum_BUFF_ITEM;
    st.val = std::stoi(std::string{id.begin(), id.end()});
    st.sub_val = 1;
    ccs.payload.push_back(std::make_unique<StateT>(st));
  }

  return;
}

void character_logic_system::run_unsit_chair(game_character &g_character) {
  if (g_character.chair.has_value()) {
    run_stand_action(g_character);
    if (&g_character == &character_game_instance::self) {
      StateT st;
      st.state = fbs::StateEnum_BUFF_ITEM;
      auto tmp = std::string{
          g_character.chair->id.begin(),
          g_character.chair->id.end(),
      };
      st.val = std::stoi(tmp);
      st.sub_val = 0;
      ccs.payload.push_back(std::make_unique<StateT>(st));
    }
  }
  g_character.chair = std::nullopt;
  return;
}

void character_logic_system::run_item(game_character &g_character) {
  if (!character_item_input.empty()) {
    for (const auto &input : character_item_input) {
      auto item_id = input.val;
      std::u16string item_id2{item_id.begin(), item_id.end()};
      auto itm = package_game_instance::load_item(item_id2);
      if (itm == nullptr) {
        return;
      }
      item_game_instance::use_item(*itm);
    }
  }
  return;
}

std::u16string
character_logic_system::load_morph_type(game_character &g_character) {
  auto action_type = load_action_type(g_character);
  switch (action_type) {
  case action_enum::stand: {
    return u"stand";
  }
  case action_enum::alert: {
    return u"stand";
  }
  case action_enum::walk: {
    return u"walk";
  }
  case action_enum::prone: {
    return u"prone";
  }
  case action_enum::jump: {
    return u"jump";
  }
  case action_enum::climb: {
    return g_character.action;
  }
  default: {
    return u"jump";
    break;
  }
  }
  return u"";
}

void character_logic_system::run_being_hit(float x, uint64_t num) {
  auto &sf = character_game_instance::self;
  character_logic_system::self_fh = 0;
  character_logic_system::self_lr = 0;

  auto action_type = character_logic_system::load_action_type(sf);
  switch (action_type) {
  case character_logic_system::action_enum::stand:
  case character_logic_system::action_enum::alert:
  case character_logic_system::action_enum::walk:
  case character_logic_system::action_enum::climb:
  case character_logic_system::action_enum::prone: {
    character_logic_system::run_action(sf, u"jump");
    break;
  }
  case character_logic_system::action_enum::sit: {
    character_logic_system::run_unsit_chair(sf);
    character_logic_system::run_action(sf, u"jump");
    break;
  }
  default: {
    break;
  }
  }
  character_logic_system::self_invincible_cooldown = window::dt_now + 2000;
  character_logic_system::self_alert_cooldown = window::dt_now + 5000;

  const auto speed = 270.0f;
  if (sf.pos.x > x) {
    if (character_logic_system::self_hspeed > 0 &&
        character_logic_system::self_hspeed <= speed) {
      character_logic_system::self_hspeed = speed;
    } else {
      character_logic_system::self_hspeed += speed;
    }
  } else {
    if (character_logic_system::self_hspeed <= 0 &&
        character_logic_system::self_hspeed > -speed) {
      character_logic_system::self_hspeed = -speed;
    } else {
      character_logic_system::self_hspeed -= speed;
    }
  }
  if (character_logic_system::self_vspeed < 0) {
    if (character_logic_system::self_vspeed > -speed) {
      character_logic_system::self_vspeed = -speed;
    }
  } else {
    character_logic_system::self_vspeed -= speed;
  }
  character_stat_game_instance::hp_point -= num;
  if (character_stat_game_instance::hp_point <= 0) {
    character_logic_system::run_die_action(sf);
  } else {
    character_logic_system::run_face_action(sf, u"hit");
  }
  return;
}
