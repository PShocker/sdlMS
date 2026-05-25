#include "mob_logic_system.h"
#include "character_logic_system.h"
#include "src/client/game/game_mob.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include "wz/Wz.h"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <flat_map>
#include <ranges>
#include <string>

std::optional<SDL_FRect> mob_logic_system::load_rect(const game_mob &g_mob) {
  auto mob_node = mob_game_instance::load_link_mob_node(g_mob.id);
  auto mob_action_node = mob_node->get_child(g_mob.action);
  bool zigzag = mob_action_node->get_child(u"zigzag") == nullptr ? false : true;
  int32_t canvas_count = mob_action_node->children_count();
  if (zigzag) {
    canvas_count -= 1;
  }
  std::string frame_index;
  if (zigzag && g_mob.ani_index >= canvas_count) {
    frame_index =
        std::to_string(canvas_count - 2 - (g_mob.ani_index % canvas_count));
  } else {
    frame_index = std::to_string(g_mob.ani_index);
  }
  auto texture_node = mob_action_node->get_child(frame_index);
  if (texture_node->get_child(u"lt") == nullptr) {
    return std::nullopt;
  }
  auto lt = wz_resource::load_fpoint(texture_node->get_child(u"lt"));
  auto rb = wz_resource::load_fpoint(texture_node->get_child(u"rb"));
  SDL_FRect rect{
      .x = lt.x,
      .y = lt.y,
      .w = rb.x - lt.x,
      .h = rb.y - lt.y,
  };
  rect.x += g_mob.pos.x;
  rect.y += g_mob.pos.y;
  if (g_mob.flip == 1) {
    rect.x += 2 * (g_mob.pos.x - rect.x) - rect.w;
  }
  return rect;
}

void mob_logic_system::run_collision() {
  if (character_logic_system::self_invincible_cooldown >= window::dt_now) {
    return;
  }
  auto &self = character_game_instance::self;
  if (self.action == u"dead") {
    return;
  }
  std::flat_map<float, game_mob> mobs;
  for (auto &m : mob_game_instance::data | std::views::values) {
    if (m.mob.hp <= 0) {
      continue;
    }
    auto m_r = load_rect(m.mob).value();
    auto c_r = character_logic_system::load_rect(self);
    if (SDL_HasRectIntersectionFloat(&m_r, &c_r)) {
      auto dx = self.pos.x - m.mob.pos.x;
      auto dy = self.pos.y - m.mob.pos.y;
      auto dis = dx * dx + dy * dy;
      mobs[dis] = m.mob;
    }
  }
  if (!mobs.empty()) {
    auto o_character = self;
    auto &mob = mobs.begin()->second;
    character_logic_system::self_fh = 0;
    character_logic_system::self_lr = 0;
    auto action_type = character_logic_system::load_action_type(self);
    switch (action_type) {
    case character_logic_system::action_enum::stand:
    case character_logic_system::action_enum::alert:
    case character_logic_system::action_enum::walk:
    case character_logic_system::action_enum::climb:
    case character_logic_system::action_enum::prone: {
      character_logic_system::run_action(self, u"jump");
    }
    default: {
      break;
    }
    }
    character_logic_system::self_invincible_cooldown = window::dt_now + 2000;
    character_logic_system::self_alert_cooldown = window::dt_now + 5000;

    const auto speed = 270.0f;
    if (self.pos.x > mob.pos.x) {
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
    character_stat_game_instance::hp_point -= 100;
    if (character_stat_game_instance::hp_point <= 0) {
      character_logic_system::run_die_action(self);
    }
    character_logic_system::run_network_sync(self, o_character);
  }
}

bool mob_logic_system::run_animate(game_mob &g_mob) {
  bool r = false;
  if (!g_mob.ani_animate) {
    return r;
  }
  auto delta = window::delta_time;
  uint32_t size = 0;
  g_mob.ani_time += delta;
  //   获取动作延迟时间
  uint32_t delay = 100;

  auto node =
      mob_game_instance::load_link_mob_node(g_mob.id)->get_child(g_mob.action);
  bool zigzag = node->get_child(u"zigzag") == nullptr ? false : true;
  int32_t canvas_count = node->children_count();
  if (zigzag) {
    canvas_count -= 1;
  }
  std::string frame_index;
  if (zigzag && g_mob.ani_index >= canvas_count) {
    frame_index =
        std::to_string(canvas_count - 2 - (g_mob.ani_index % canvas_count));
  } else {
    frame_index = std::to_string(g_mob.ani_index);
  }
  if (auto canvas_node = node->get_child(frame_index)) {
    if (auto delay_node = canvas_node->get_child(u"delay")) {
      if (delay_node->type == wz::Type::String) {
        auto delay2 =
            static_cast<wz::Property<std::u16string> *>(delay_node)->get();
        delay = std::stoi(std::string{delay2.begin(), delay2.end()});
      } else {
        delay = static_cast<wz::Property<int32_t> *>(delay_node)->get();
      }
    }
  }

  if (g_mob.ani_time >= delay) {
    g_mob.ani_index += 1;
    int32_t size = !zigzag ? canvas_count : (canvas_count - 1) * 2 + 1;
    r = g_mob.ani_index >= size;
    g_mob.ani_index = g_mob.ani_index % size;
    g_mob.ani_time = 0;
  }
  return r;
}

void mob_logic_system::run_logic() {
  for (auto &m : mob_game_instance::data | std::views::values) {
    for (const auto &[k, v] : m.logics) {
      switch (k) {
      case fbs::MobLogicType_Movement: {
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
          float t = ((m.mob.pos.x - mv.x1) * dx + (m.mob.pos.y - mv.y1) * dy) /
                    length_sq;
          t = std::clamp(t, 0.0f, 1.0f);
          per += t;
        }
        per = std::min(per, 1.0f);

        auto per_x = mv.x1 + (mv.x2 - mv.x1) * per;
        auto per_y = mv.y1 + (mv.y2 - mv.y1) * per;
        m.mob.pos.x = per_x;
        m.mob.pos.y = per_y;
        m.mob.page = mv.page;
        if (per == 1.0f) {
          m.logics.erase(m.logics.begin());
        }
        break;
      }
      case fbs::MobLogicType_Flip: {
        if (v.empty()) {
          break;
        }
        auto f = *v.back().AsFlip();
        m.mob.flip = f.flip;
        v.clear();
        break;
      }
      case fbs::MobLogicType_Action: {
        if (v.empty()) {
          break;
        }
        auto a = *v.back().AsAction();
        m.mob.action = std::u16string{a.action.begin(), a.action.end()};
        m.mob.ani_index = 0;
        m.mob.ani_time = 0;
        m.mob.ani_animate = a.action_animate;
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

void mob_logic_system::run_state_machine(game_mob &g_mob) {
  run_animate(g_mob);
}

bool mob_logic_system::run() {
  for (auto &m : mob_game_instance::data | std::views::values) {
    run_state_machine(m.mob);
  }
  run_collision();
  run_logic();
  return true;
}