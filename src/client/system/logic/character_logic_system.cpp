#include "character_logic_system.h"
#include "SDL3/SDL_rect.h"
#include "drop_logic_system.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_effect.h"
#include "src/client/game/game_mob.h"
#include "src/client/game_instance/afterimage_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/effect_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/ladderrope_game_instance.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/game_instance/portal_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/game_instance/seat_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/mob_logic_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/physic/physic.h"
#include "src/common/request/client_request.h"
#include "wz/Property.h"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <flat_map>
#include <flat_set>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

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

std::vector<character_logic_system::attack_data>
character_logic_system::run_attack_check(game_character &g_character,
                                         SDL_FRect g_r) {
  std::vector<attack_data> v;
  std::flat_map<uint32_t, attack_data> m;
  auto &g_pos = g_character.pos;
  for (const auto [k, v] : mob_game_instance::data) {
    auto &m_pos = v.mob.pos;
    auto m_r = mob_logic_system::load_rect(v.mob).value();
    if (SDL_HasRectIntersectionFloat(&m_r, &g_r)) {
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
  return v;
}

std::vector<character_logic_system::attack_data>
character_logic_system::run_shoot_check2(game_character &g_character) {
  std::vector<attack_data> v;
  return v;
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
    if (self_sit_cooldown >= window::dt_now) {
      run_stand_action(g_character);
      self_sit_cooldown = window::dt_now + 120;
      return false;
    }
  }
  return true;
}

bool character_logic_system::run_skill(game_character &g_character) {
  if (g_character.abnormals.contains(
          game_character::abnormal_state_type::dizz)) {
    return false;
  }
  if (!g_character.weapon.has_value()) {
    return false;
  }
  if (self_attack_cooldown > window::dt_now) {
    return false;
  }
  if (!character_skill_input.empty()) {
    const auto &s_id = *character_skill_input.begin();
    std::u16string s_id2{s_id.begin(), s_id.end()};
    auto skill_node = skill_game_instance::load_skill_node(s_id2);
    if (auto action_node = skill_node->get_child(u"action")) {
      auto action_count = action_node->children_count();
      std::uniform_int_distribution<> dis(0, action_count - 1);
      auto &gen = random_game_instance::gen;
      auto action = dis(gen);
      auto action2 = std::to_string(action);
      auto action3 = action_node->get_child(action2);
      auto action4 =
          static_cast<wz::Property<std::u16string> *>(action3)->get();
      run_action(g_character, action4);
    }
    auto skill_level = job_skill_game_instance::load_self_skill_level(s_id2);
    auto skill_type = skill_game_instance::load_skill_type(s_id2, skill_level);

    ClientCharacterAttackT t;
    ClientCharacterSkillT ckt;
    ckt.ski_id = std::stoi(s_id);

    switch (skill_type) {
    case skill_game_instance::attack: {
      auto g_r = skill_game_instance::load_skill_rect(s_id2, skill_level);
      auto atk_mobs = run_attack_check(g_character, g_r);
      if (!atk_mobs.empty()) {
        auto skill_level_node =
            skill_game_instance::load_skill_level_node(s_id2, skill_level);
        auto atk_mob_count = static_cast<wz::Property<int32_t> *>(
                                 skill_level_node->get_child(u"mobCount"))
                                 ->get();
        auto atk_count = static_cast<wz::Property<int32_t> *>(
                             skill_level_node->get_child(u"attackCount"))
                             ->get();
        for (uint32_t i = 0; i < atk_mobs.size() && i < atk_mob_count; i++) {
          auto delay = skill_game_instance::load_beat_time(g_character);
          CharacterAttackT ct;
          ct.mob_index = atk_mobs[i].mob.index;
          ct.attack = std::make_unique<AttackT>();
          ct.attack->num = 1;
          ct.attack->delay = delay + i * 60;
          ct.attack->x = atk_mobs[i].x;
          ct.attack->y = atk_mobs[i].y;
          ct.afterimage = false;
          ct.left = g_character.pos.x < atk_mobs[i].mob.pos.x;
          t.payload.push_back(std::make_unique<CharacterAttackT>(ct));
        }
        character_game_instance::load_character_attack(t.payload, g_character);
        client_request::character_attack_request(t);
        for (const auto &a : t.payload) {
          CharacterSkillT c;
          c.delay = a->attack->delay;
          c.mob = a->mob_index;
          c.x = a->attack->x;
          c.y = a->attack->y;
          ckt.payload.push_back(std::make_unique<CharacterSkillT>(c));
        }
      }
      break;
    }
    case skill_game_instance::move: {
      break;
    }
    case skill_game_instance::buff: {
      break;
    }
    }
    character_game_instance::load_character_skill(
        ckt.ski_id, ckt.payload, character_game_instance::self);
    client_request::character_skill_request(ckt);
    return true;
  }
  return false;
}

std::vector<character_logic_system::attack_data>
character_logic_system::run_shoot_check(game_character &g_character) {
  SDL_FRect r{-20, -20, 20, 20};
  return run_attack_check(g_character, r);
}

bool character_logic_system::run_attack(game_character &g_character) {
  if (g_character.abnormals.contains(
          game_character::abnormal_state_type::dizz)) {
    return false;
  }
  if (!g_character.weapon.has_value()) {
    return false;
  }
  if (self_attack_cooldown > window::dt_now) {
    return false;
  }
  if (character_action_input.contains("attack")) {
    auto g_action = load_action_type(g_character);
    auto g_weapon = g_character.weapon->id;
    auto g_weapon_info = equip_game_instance::load_equip_info(g_weapon);
    auto weapon_type = equip_game_instance::load_weapon_type(g_character);
    std::vector<character_logic_system::attack_data> atk_mobs;
    bool shoot = false;
    static const std::flat_set<equip_game_instance::weapon_type> shoot_weapons =
        {
            equip_game_instance::BOW,
            equip_game_instance::CROSSBOW,
            equip_game_instance::CLAW,
            equip_game_instance::GUN,
        };
    static const std::flat_map<equip_game_instance::weapon_type,
                               std::vector<std::u16string>>
        weapon_attack_action = {
            {equip_game_instance::S1A1M1D,
             {u"stabO1", u"stabO2", u"swingO1", u"swingO2", u"swingO3"}},
            {equip_game_instance::SPEAR, {u"stabT1", u"swingP1"}},
            {equip_game_instance::BOW, {u"shoot1"}},
            {equip_game_instance::CROSSBOW, {u"shoot1"}},
            {equip_game_instance::S2A2M2,
             {u"stabO1", u"stabO2", u"swingT1", u"swingT2", u"swingT3"}},
            {equip_game_instance::WAND, {u"swingO1", u"swingO2"}},
            {equip_game_instance::CLAW, {u"swingO1", u"swingO2"}},
            {equip_game_instance::GUN, {u"shot"}},
        };
    static const std::flat_map<equip_game_instance::weapon_type,
                               std::vector<std::u16string>>
        weapon_attack_action2 = {
            {equip_game_instance::BOW, {u"swingT1", u"swingT3"}},
            {equip_game_instance::CROSSBOW, {u"swingT1", u"stabT1"}},
            {equip_game_instance::CLAW, {u"stabO1", u"stabO2"}},
            {equip_game_instance::GUN, {u"swingP1", u"stabT2"}},
        };
    switch (g_action) {
    case action_enum::stand:
    case action_enum::alert:
    case action_enum::walk: {
      self_hspeed = 0;
    }
    case action_enum::jump: {
      auto &gen = random_game_instance::gen;
      bool shoot_weapon = shoot_weapons.contains(weapon_type);
      const std::vector<std::u16string> *actions;
      if (shoot_weapon) {
        atk_mobs = run_shoot_check(g_character);
        if (!atk_mobs.empty()) {
          actions = &weapon_attack_action2.at(weapon_type);
        } else {
          shoot = true;
          actions = &weapon_attack_action.at(weapon_type);
        }
      } else {
        actions = &weapon_attack_action.at(weapon_type);
      }
      std::uniform_int_distribution<> dis(0, actions->size() - 1);
      auto selected = actions->at(dis(gen));
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
    SDL_FRect g_r = afterimage_game_instance::load_rect(g_character).value();
    if (atk_mobs.empty()) {
      atk_mobs = run_attack_check(g_character, g_r);
    }
    if (!atk_mobs.empty()) {
      auto atk_mob = atk_mobs[0];
      uint64_t delay;
      if (!shoot) {
        delay = afterimage_game_instance::load_beat_time(g_character);
      } else {
        // shoot
        delay = afterimage_game_instance::load_beat_time(g_character);
        float hspeed = 100.0f;
        auto dx = atk_mob.mob.pos.x - g_character.pos.x;
        delay += dx / hspeed;
      }
      ClientCharacterAttackT t;
      CharacterAttackT ct;
      ct.mob_index = atk_mob.mob.index;
      ct.attack = std::make_unique<AttackT>();
      ct.attack->num = 1;
      ct.attack->delay = delay;
      ct.attack->x = atk_mob.x;
      ct.attack->y = atk_mob.y;
      ct.afterimage = true;
      ct.left = g_character.pos.x < atk_mob.mob.pos.x;
      t.payload.push_back(std::make_unique<CharacterAttackT>(ct));

      character_game_instance::load_character_attack(t.payload, g_character);
      client_request::character_attack_request(t);
    }
    self_alert_cooldown = window::dt_now + 5000;
    return true;
  }
  return false;
}

bool character_logic_system::run_portal(game_character &g_character) {
  if (g_character.abnormals.contains(
          game_character::abnormal_state_type::dizz)) {
    return false;
  }
  if (character_action_input.contains("up")) {
    if (self_portal_cooldown <= window::dt_now) {
      for (const auto &por : portal_game_instance::data | std::views::values) {
        if (por.tm == 999999999) {
          continue;
        }
        if (por.pt == 1 || por.pt == 2) {
          const auto &g_pos = g_character.pos;
          const auto &p_pos = por.pos;
          if (g_pos.x == std::clamp(g_pos.x, p_pos.x - 40, p_pos.x + 40) &&
              g_pos.y == std::clamp(g_pos.y, p_pos.y - 50, p_pos.y + 50)) {
            if (por.tm != scene_system_instance::map_id) {
              // need to change map
              scene_system_instance::enter_prepare(por.tm, por.tn, 0);
              self_portal_cooldown = window::dt_now + 1500;
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
              return true;
            }
          }
        }
      }
    }
  }
  return false;
}

void character_logic_system::run_face(game_character &g_character) {
  // g_character.face;
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

void character_logic_system::run_network_action_sync(
    game_character &g_character, game_character &o_character) {

  static bool last_action;
  static uint64_t last_send_time = 0;
  const int32_t MIN_SEND_INTERVAL_MS = 33;

  bool action_changed =
      (o_character.action != g_character.action ||
       o_character.action_animate != g_character.action_animate);

  if (last_action) {
    action_changed = true;
    last_action = false;
    last_send_time = window::dt_now - MIN_SEND_INTERVAL_MS;
  }

  if (!action_changed)
    return;

  // 节流：频率限制
  if (window::dt_now - last_send_time >= MIN_SEND_INTERVAL_MS) {
    ClientCharacterLogicT t;
    ActionT a;
    a.action =
        std::string{g_character.action.begin(), g_character.action.end()};
    a.action_animate = g_character.action_animate;
    a.action_index = g_character.action_index;
    t.payload.Set(a);
    client_request::character_logic_request(t);
    last_send_time = window::dt_now;
    last_action = false;
  } else {
    last_action = true;
  }
}

void character_logic_system::run_network_flip_sync(
    game_character &g_character, game_character &o_character) {
  static bool last_flip;
  static uint64_t last_send_time = 0;
  const int32_t MIN_SEND_INTERVAL_MS = 33;

  bool flip_changed = o_character.flip != g_character.flip;
  if (last_flip) {
    flip_changed = true;
    last_flip = false;
    last_send_time = window::dt_now - MIN_SEND_INTERVAL_MS;
  }

  if (!flip_changed)
    return;

  // 节流：频率限制
  if (window::dt_now - last_send_time >= MIN_SEND_INTERVAL_MS) {
    ClientCharacterLogicT t;
    FlipT f;
    f.flip = g_character.flip;
    t.payload.Set(f);
    client_request::character_logic_request(t);
    last_send_time = window::dt_now;
    last_flip = false;
  } else {
    last_flip = true;
  }
}

void character_logic_system::run_network_movement_sync(
    game_character &g_character, game_character &o_character) {
  static bool last_movement;

  static uint64_t last_send_time = 0;
  const int32_t MIN_SEND_INTERVAL_MS = 33;

  bool position_changed = (o_character.pos.x != g_character.pos.x ||
                           o_character.pos.y != g_character.pos.y);

  if (last_movement) {
    position_changed = true;
    last_movement = false;
    last_send_time = window::dt_now - MIN_SEND_INTERVAL_MS;
  }

  if (!position_changed)
    return;

  // 构造当前 movement
  MovementT mv;
  mv.x1 = o_character.pos.x;
  mv.y1 = o_character.pos.y;
  mv.x2 = g_character.pos.x;
  mv.y2 = g_character.pos.y;
  mv.page = g_character.page;
  mv.time = std::min(window::delta_time, MIN_SEND_INTERVAL_MS);

  // 节流：频率限制
  if (window::dt_now - last_send_time >= MIN_SEND_INTERVAL_MS) {
    ClientCharacterLogicT req;
    req.payload.Set(mv);
    client_request::character_logic_request(req);
    last_send_time = window::dt_now;
    last_movement = false;
  } else {
    last_movement = true;
  }
}

void character_logic_system::run_network_sync(game_character &g_character,
                                              game_character &o_character) {
  run_network_movement_sync(g_character, o_character);
  run_network_flip_sync(g_character, o_character);
  run_network_action_sync(g_character, o_character);
}

character_logic_system::action_enum
character_logic_system::load_action_type(game_character &g_character) {
  if (g_character.skill.has_value()) {
    return action_enum::skill;
  }
  const static std::flat_map<std::u16string, action_enum> map_name = {
      {u"stand1", action_enum::stand},   {u"stand2", action_enum::stand},
      {u"alert", action_enum::alert},    {u"walk1", action_enum::walk},
      {u"walk2", action_enum::walk},     {u"prone", action_enum::prone},
      {u"jump", action_enum::jump},      {u"ladder", action_enum::climb},
      {u"rope", action_enum::climb},     {u"proneStab", action_enum::attack},
      {u"stabO1", action_enum::attack},  {u"stabO2", action_enum::attack},
      {u"swingO1", action_enum::attack}, {u"swingO2", action_enum::attack},
      {u"swingO3", action_enum::attack}, {u"stabT1", action_enum::attack},
      {u"swingP1", action_enum::attack}, {u"shoot1", action_enum::attack},
      {u"swingT1", action_enum::attack}, {u"swingT2", action_enum::attack},
      {u"swingT3", action_enum::attack}, {u"shot", action_enum::attack},
      {u"stabT2", action_enum::attack},
  };
  return map_name.at(g_character.action);
};

void character_logic_system::run_state_machine(game_character &g_character) {
  auto o_character = g_character;
  auto g_action = load_action_type(g_character);
  run_face(g_character);
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
    break;
  }
  case action_enum::jump: {
    run_flip(g_character);
    if (!run_fall(g_character)) {
      // 刚落地后，瞬间动作不一定是stand，需要再进行一次状态机
      run_stand_action(g_character);
      run_state_machine(g_character);
    }
    if (run_attack(g_character)) {
      break;
    }
    if (run_climb(g_character)) {
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
    break;
  }
  case action_enum::sit: {
    if (!run_sitting(g_character)) {
      run_state_machine(g_character);
    }
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
      run_state_machine(g_character);
    }
    break;
  }
  default: {
    std::abort();
  }
  }
  run_network_sync(g_character, o_character);
}

void character_logic_system::run_others_logic() {
  for (auto &c : character_game_instance::others | std::views::values) {
    for (const auto &[k, v] : c.logics) {
      switch (k) {
      case fbs::CharacterLogicType_Movement: {
        if (v.empty()) {
          break;
        }
        auto &l = v[0];
        auto mv = *l.AsMovement();
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
          c.logics.erase(c.logics.begin());
        }
        break;
      }
      case fbs::CharacterLogicType_Flip: {
        if (v.empty()) {
          break;
        }
        auto f = *v.back().AsFlip();
        c.g_character.flip = f.flip;
        v.clear();
        break;
      }
      case fbs::CharacterLogicType_Action: {
        if (v.empty()) {
          break;
        }
        auto a = *v.back().AsAction();
        c.g_character.action = std::u16string{a.action.begin(), a.action.end()};
        c.g_character.action_index = a.action_index;
        c.g_character.action_animate = a.action_animate;
        c.g_character.action_time = 0;
        v.clear();
        break;
      }
      default: {
        std::abort();
      }
      }
    }
  }
}

void character_logic_system::run_others_animate() {
  for (auto &c : character_game_instance::others | std::views::values) {
    auto &g_character = c.g_character;
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
    default: {
      break;
    }
    }
  }
}

void character_logic_system::run_others() {
  run_others_animate();
  run_others_logic();
}

// 人物状态机
bool character_logic_system::run() {
  run_others();
  run_state_machine(character_game_instance::self);
  return true;
}