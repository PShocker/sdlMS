#include "npc_logic_system.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_chatballoon.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/physic/physic.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <cstdint>
#include <flat_set>
#include <optional>
#include <random>
#include <ranges>
#include <string>

bool npc_logic_system::run_animate(game_npc &g_npc) {
  bool r = false;
  g_npc.ani_time += window::delta_time;
  auto npc_node = npc_game_instance::load_link_npc_node(g_npc.id);
  auto action_node = npc_node->get_child(g_npc.action);

  auto texture_node = action_node->get_child(std::to_string(g_npc.ani_index));

  int32_t delay = 100;
  auto delay_node = texture_node->get_child(u"delay");
  if (delay_node != nullptr) {
    delay = static_cast<wz::Property<int> *>(delay_node)->get();
  }
  auto child = *action_node->get_children();
  int32_t canvas_count = 0;
  for (auto [k, v] : child) {
    if (u'0' <= k[0] && k[0] <= u'9') {
      canvas_count++;
    }
  }
  if (g_npc.ani_time >= delay) {
    g_npc.ani_time = 0;
    g_npc.ani_index++;
    r = g_npc.ani_index >= canvas_count;
    g_npc.ani_index = g_npc.ani_index % canvas_count;
  }
  return r;
}

bool npc_logic_system::run_duration(game_npc &g_npc) {
  if (g_npc.duration > window::dt_now) {
    return false;
  }
  auto npc_node = npc_game_instance::load_link_npc_node(g_npc.id);
  auto child = *npc_node->get_children();
  child.erase(u"info");
  std::uniform_int_distribution<> dis(0, child.size() - 1);

  // flat_map 支持随机访问
  auto &gen = random_game_instance::gen;
  auto it = std::next(child.begin(), dis(gen));
  auto random_key = it->first;

  g_npc.action = random_key;
  auto action_type = load_action_type(g_npc);
  if (action_type == action_enum::move) {
    std::bernoulli_distribution dist(0.5); // 50% 概率为 true
    bool random_bool = dist(gen);
    g_npc.hforce = random_bool ? 1400 : -1400;
    g_npc.hspeed = 0.0f;
  } else {
    g_npc.hforce = std::nullopt;
    g_npc.hspeed = std::nullopt;
  }
  g_npc.duration = window::dt_now + 1000;
  return true;
}

bool npc_logic_system::run_chatballoon(game_npc &g_npc) {
  if (g_npc.chatballoon.has_value()) {
    auto &chatballoon = g_npc.chatballoon.value();
    if (chatballoon.destory < window::dt_now) {
      g_npc.chatballoon = std::nullopt;
    }
  } else {
    std::uniform_int_distribution<int> dist(5000, 10000);
    auto &gen = random_game_instance::gen;
    // 生成随机数
    int random_number = dist(gen);

    auto npc_node = npc_game_instance::load_link_npc_node(g_npc.id);
    auto npc_info = npc_node->get_child(u"info");
    std::flat_set<std::u16string> speaks;
    if (npc_info) {
      if (auto speak_node = npc_info->get_child(u"speak")) {
        for (auto [k, v] : *speak_node->get_children()) {
          auto str = static_cast<wz::Property<std::u16string> *>(v[0])->get();
          speaks.insert(str);
        }
      }
    }

    game_chatballoon c;
    c.destory = window::dt_now + random_number;
    // c.path = u"25";
    // c.path = u"20";
    c.path = u"0";
    c.pos = {0, 0};
    c.size = 14;
    c.color = {156, 91, 97, 255};

    auto view =
        g_npc.id | std::views::drop_while([](char16_t c) { return c == u'0'; });

    std::u16string result(view.begin(), view.end());
    auto str_node = wz_resource::string->find(u"Npc.img/" + result);
    auto child = *str_node->get_children();
    std::vector<std::u16string> intersection;

    for (const auto &key : speaks) {
      if (child.contains(key)) {
        intersection.push_back(key);
      }
    }
    if (!intersection.empty()) {
      std::uniform_int_distribution<int> dist2(0, intersection.size() - 1);
      int random_number2 = dist2(gen);
      str_node = child.at(intersection[random_number2])[0];
      c.text = static_cast<wz::Property<std::u16string> *>(str_node)->get();
      g_npc.chatballoon = c;
    }
  }
  return true;
}

bool npc_logic_system::run_move(game_npc &g_npc) {
  // 移动
  auto delta_time = window::delta_time / 1000.0f;
  float vspeed = 0;
  auto r = physic::walk(g_npc.pos, delta_time, g_npc.hspeed.value(), vspeed,
                        g_npc.hforce.value(), -100, 100, 800, false, g_npc.fh,
                        std::nullopt, foothold_game_instance::data);
  return r;
}

npc_logic_system::action_enum
npc_logic_system::load_action_type(game_npc &g_npc) {
  if (g_npc.action == u"move") {
    return action_enum::move;
  }
  return action_enum::stand;
}

bool npc_logic_system::run() {
  for (auto &m : npc_game_instance::data) {
    for (auto &g_npc : m) {
      run_chatballoon(g_npc);
      auto action_type = load_action_type(g_npc);
      switch (action_type) {
      case action_enum::move: {
        run_move(g_npc);
        break;
      }
      default: {
        break;
      }
      }
      if (run_animate(g_npc)) {
        run_duration(g_npc);
      }
    }
  }
  return true;
}

std::optional<std::u16string> npc_logic_system::cursor_in() {
  std::optional<std::u16string> r;
  auto &camera = camera_game_instance::camera;
  for (auto &m : npc_game_instance::data) {
    for (auto &g_npc : m) {
      auto npc_node = npc_game_instance::load_link_npc_node(g_npc.id);
      auto action_node = npc_node->get_child(g_npc.action);

      auto index = std::to_string(g_npc.ani_index);
      npc_node = action_node->get_child(index);

      auto texture = wz_resource::load_texture(npc_node);
      auto origin = wz_resource::load_fpoint(npc_node->get_child(u"origin"));
      SDL_FRect pos_rect = {
          .x = g_npc.pos.x - origin.x,
          .y = g_npc.pos.y - origin.y,
          .w = static_cast<float>(texture->w),
          .h = static_cast<float>(texture->h),
      };
      if (g_npc.flip == 1) {
        pos_rect.x = g_npc.pos.x;
        pos_rect.x = (pos_rect.x - (texture->w - origin.x));
      }
      pos_rect.x -= camera.x;
      pos_rect.y -= camera.y;
      const auto &pos = window::mouse_pos;
      if (SDL_PointInRectFloat(&pos, &pos_rect)) {
        return g_npc.id;
      }
    }
  }
  return r;
}