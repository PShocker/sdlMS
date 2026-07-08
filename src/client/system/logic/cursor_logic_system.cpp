#include "cursor_logic_system.h"
#include "SDL3/SDL_mouse.h"
#include "character_logic_system.h"
#include "npc_logic_system.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/quest_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/game_instance/shop_game_instance.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/character_info_ui_system.h"
#include "src/client/system/ui/character_stat_ui_system.h"
#include "src/client/system/ui/context_menu_ui_system.h"
#include "src/client/system/ui/craft_ui_system.h"
#include "src/client/system/ui/equip_ui_system.h"
#include "src/client/system/ui/minimap_ui_system.h"
#include "src/client/system/ui/npc_dlg_ui_system.h"
#include "src/client/system/ui/package_ui_system.h"
#include "src/client/system/ui/revive_ui_system.h"
#include "src/client/system/ui/shop_ui_system.h"
#include "src/client/system/ui/skill_ui_system.h"
#include "src/client/system/ui/statusbar_ui_system.h"
#include "src/client/system/ui/storage_ui_system.h"
#include "src/client/system/ui/trade_ui_system.h"
#include "src/client/system/ui/worldmap_ui_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/physic/physic.h"
#include "src/common/request/client_request.h"
#include "src/common/script/script.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include "wz/Wz.h"
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <optional>
#include <string>

using namespace fbs;

std::optional<game_npc> cursor_logic_system::cursor_npc() {
  const auto &pos = window::mouse_pos;
  auto &camera = camera_game_instance::camera;
  for (auto &m : npc_game_instance::data) {
    for (auto &g_npc : m) {
      auto quests = quest_game_instance::load_npc_quest(g_npc.id);
      if (!quests.empty()) {
        auto quest_r = npc_game_instance::load_quest_rect(g_npc).value();
        quest_r.x -= camera.x;
        quest_r.y -= camera.y;
        if (SDL_PointInRectFloat(&pos, &quest_r)) {
          return g_npc;
        }
      }
      auto npc_rect = npc_game_instance::load_rect(g_npc);
      npc_rect.x -= camera.x;
      npc_rect.y -= camera.y;
      if (SDL_PointInRectFloat(&pos, &npc_rect)) {
        return g_npc;
      }
    }
  }
  return std::nullopt;
}

bool cursor_logic_system::run_package_motion() {
  auto index = package_ui_system::load_mouse_index();
  if (!index.has_value()) {
    return false;
  }
  auto active_tab = package_ui_system::active_tab;
  const auto &r = package_game_instance::data[active_tab];
  if (!r[index.value()].has_value()) {
    return false;
  }
  return true;
}

void cursor_logic_system::run_cursor_action(const std::u16string &action) {
  if (cursor_game_instance::cursor_type != action) {
    cursor_game_instance::cursor_type = action;
    cursor_game_instance::cursor_index = 0;
    cursor_game_instance::cursor_time = 0;
  }
}

bool cursor_logic_system::run_default() {
  if (cursor_game_instance::cursor_hand.has_value()) {
    run_cursor_action(u"11");
    return true;
  }
  if (cursor_game_instance::cursor_ui == nullptr) {
    if (!(window::mouse_state & SDL_BUTTON_LMASK)) {
      auto npc = cursor_npc();
      if (npc.has_value()) {
        run_cursor_action(u"1");
        return true;
      }
    }
  }
  if (cursor_game_instance::cursor_ui == package_ui_system::render) {
    if (run_package_motion()) {
      run_cursor_action(u"5");
      return true;
    }
  }
  if (window::mouse_state & SDL_BUTTON_LMASK) {
    run_cursor_action(u"12");
  } else {
    run_cursor_action(u"0");
  }

  return true;
}

void cursor_logic_system::run_cursor_ui() {
  cursor_game_instance::cursor_ui = nullptr;
  for (auto &fn : system::render_systems) {
    if (fn == minimap_ui_system::render) {
      if (minimap_ui_system::cursor_in()) {
        cursor_game_instance::cursor_ui = fn;
      }
    } else if (fn == statusbar_ui_system::render) {
      if (statusbar_ui_system::cursor_in()) {
        cursor_game_instance::cursor_ui = fn;
      }
    } else if (fn == worldmap_ui_system::render) {
      if (worldmap_ui_system::cursor_in()) {
        cursor_game_instance::cursor_ui = fn;
      }
    } else if (fn == equip_ui_system::render) {
      if (equip_ui_system::cursor_in()) {
        cursor_game_instance::cursor_ui = fn;
      }
    } else if (fn == package_ui_system::render) {
      if (package_ui_system::cursor_in()) {
        cursor_game_instance::cursor_ui = fn;
      }
    } else if (fn == character_stat_ui_system::render) {
      if (character_stat_ui_system::cursor_in()) {
        cursor_game_instance::cursor_ui = fn;
      }
    } else if (fn == character_info_ui_system::render) {
      if (character_info_ui_system::cursor_in()) {
        cursor_game_instance::cursor_ui = fn;
      }
    } else if (fn == skill_ui_system::render) {
      if (skill_ui_system::cursor_in()) {
        cursor_game_instance::cursor_ui = fn;
      }
    } else if (fn == craft_ui_system::render) {
      if (craft_ui_system::cursor_in()) {
        cursor_game_instance::cursor_ui = fn;
      }
    } else if (fn == revive_ui_system::render) {
      if (revive_ui_system::cursor_in()) {
        cursor_game_instance::cursor_ui = fn;
      }
    } else if (fn == trade_ui_system::render) {
      if (trade_ui_system::cursor_in()) {
        cursor_game_instance::cursor_ui = fn;
      }
    } else if (fn == storage_ui_system::render) {
      if (storage_ui_system::cursor_in()) {
        cursor_game_instance::cursor_ui = fn;
      }
    } else if (fn == context_menu_ui_system::render) {
      if (context_menu_ui_system::cursor_in()) {
        cursor_game_instance::cursor_ui = fn;
      }
    } else if (fn == npc_dlg_ui_system::render) {
      if (npc_dlg_ui_system::cursor_in()) {
        cursor_game_instance::cursor_ui = fn;
      }
    } else if (fn == shop_ui_system::render) {
      if (shop_ui_system::cursor_in()) {
        cursor_game_instance::cursor_ui = fn;
      }
    }
  }
}

bool cursor_logic_system::run_animate() {
  static auto cursor_node = wz_resource::ui->find(u"Cursor.img");
  auto type_node = cursor_node->get_child(cursor_game_instance::cursor_type);
  auto child_count = type_node->children_count();
  auto index = std::to_string(cursor_game_instance::cursor_index);
  type_node = type_node->get_child(index);
  if (type_node->type == wz::Type::UOL) {
    type_node = static_cast<wz::Property<wz::WzUOL> *>(type_node)->get_uol();
  }
  int delay = 0;
  if (type_node->get_child(u"delay")) {
    delay =
        static_cast<wz::Property<int> *>(type_node->get_child(u"delay"))->get();
  }
  cursor_game_instance::cursor_time += window::delta_time;
  if (cursor_game_instance::cursor_time >= delay) {
    cursor_game_instance::cursor_time = 0;
    cursor_game_instance::cursor_index += 1;
    cursor_game_instance::cursor_index =
        cursor_game_instance::cursor_index % child_count;
  }
  return true;
}

bool cursor_logic_system::run() {
  run_animate();
  run_cursor_ui();
  run_default();
  return true;
}

bool cursor_logic_system::event_cursor_hand(SDL_Event *event) {
  switch (event->type) {
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (cursor_game_instance::cursor_ui == nullptr) {
        auto &cursor_hand = cursor_game_instance::cursor_hand;
        if (cursor_hand.has_value()) {
          switch (cursor_hand->type) {
          case cursor_game_instance::equipment: {
            break;
          }
          case cursor_game_instance::package: {
            auto active_tab = cursor_hand->val;
            if (active_tab == 0) {
              auto equip = package_game_instance::data[0][cursor_hand->sub_val];
              DropT dt;
              EquipT et;
              et.equip_id =
                  std::stoi(std::string{equip->id.begin(), equip->id.end()});
              dt.drop.Set(et);

              dt.x1 = character_game_instance::self.pos.x;
              dt.y1 = character_game_instance::self.pos.y;

              dt.x2 = character_game_instance::self.pos.x;
              dt.y2 = character_game_instance::self.pos.y - 100;

              dt.page = character_game_instance::self.page;

              auto border = map_info_game_instance::load_mr_border(
                  scene_system_instance::map_id);
              int32_t tmp_fh;
              uint8_t tmp_page;
              float tmp_hsp = 0;
              float tmp_vsp = 10000;
              SDL_FPoint tmp_fp{dt.x2, dt.y2};
              physic::fall(tmp_fp, 100000, tmp_hsp, tmp_vsp, tmp_vsp, tmp_vsp,
                           border, true, true, tmp_fh, tmp_page,
                           foothold_game_instance::data);
              dt.x2 = tmp_fp.x;
              dt.y2 = tmp_fp.y;

              std::uniform_int_distribution<uint64_t> dist;
              // 生成一个随机的 uint64_t
              dt.random_id = dist(random_game_instance::gen);

              ClientCharacterDropT cct;
              cct.map_id = scene_system_instance::map_id;
              cct.payload = std::make_unique<DropT>(dt);
              client_request::send_to_host(cct);

              cursor_game_instance::cursor_hand_net = {
                  .type = cursor_game_instance::drop,
                  .id = dt.random_id,
              };
            } else {
            }
            break;
          }
          case cursor_game_instance::skill: {
            cursor_game_instance::cursor_hand = std::nullopt;
            break;
          }
          default: {
            break;
          }
          }
        }
      }
    }
    break;
  }
  default: {
    break;
  }
  }

  return false;
}

bool cursor_logic_system::event_character(SDL_Event *event) {
  bool r = false;
  switch (event->type) {
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT && event->button.clicks == 2) {
      if (cursor_game_instance::cursor_ui != nullptr) {
        break;
      }
      auto others = character_game_instance::others.values();
      character_other_data s{.g_character = character_game_instance::self};
      others.push_back(s);
      auto &camera = camera_game_instance::camera;
      for (auto character : others) {
        auto rect = character_logic_system::load_rect(character.g_character);
        rect.x -= camera.x;
        rect.y -= camera.y;
        bool ins = SDL_PointInRectFloat(&window::mouse_pos, &rect);
        if (ins) {
          character_info_ui_system::character = character.g_character;
          character_info_ui_system::close();
          character_info_ui_system::open();
          return true;
        }
      }
    } else if (event->button.button == SDL_BUTTON_RIGHT) {
      if (cursor_game_instance::cursor_ui != nullptr) {
        break;
      }
      auto others = character_game_instance::others;
      character_other_data s{.g_character = character_game_instance::self};
      others[0] = s;
      auto &camera = camera_game_instance::camera;
      for (auto [k, v] : others) {
        auto rect = character_logic_system::load_rect(v.g_character);
        rect.x -= camera.x;
        rect.y -= camera.y;
        bool ins = SDL_PointInRectFloat(&window::mouse_pos, &rect);
        if (ins) {
          context_menu_ui_system::pos = window::mouse_pos;
          context_menu_ui_system::client_id = k;
          context_menu_ui_system::client_name = v.g_character.nametags[0].text;
          context_menu_ui_system::close();
          context_menu_ui_system::open();
          return true;
        }
      }
    }
    break;
  }
  default: {
    break;
  }
  }
  return r;
}

bool cursor_logic_system::event_npc(SDL_Event *event) {
  bool r = false;
  switch (event->type) {
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT && event->button.clicks == 2) {
      if (cursor_game_instance::cursor_ui != nullptr) {
        break;
      }
      auto npc = cursor_npc();
      if (npc.has_value()) {
        auto npc_id = npc->id;
        auto npc_type = npc_game_instance::load_npc_type(npc_id);
        switch (npc_type) {
        case npc_game_instance::npc_type::shop: {
          auto shop = shop_game_instance::load_npc_shop(npc_id);
          shop_ui_system::shop = shop;
          shop_ui_system::npc = npc.value();
          shop_ui_system::close();
          shop_ui_system::open();
          r = true;
          break;
        }
        case npc_game_instance::npc_type::script: {
          auto node = wz_resource::npc->find(npc_id + u".img");
          auto script_node = node->find(u"info/script/0/script");
          auto script_str =
              static_cast<wz::Property<std::u16string> *>(script_node)->get();
          script::fns().at(script_str)();
          r = true;
          break;
        }
        case npc_game_instance::npc_type::quest: {
          auto quest = quest_game_instance::load_npc_quest(npc_id);
          npc_dlg_ui_system::close();
          npc_dlg_ui_system::open();
          npc_dlg_ui_system::type = npc_dlg_ui_system::npc_dlg_enum::quest;
          npc_dlg_ui_system::index = 0;
          npc_dlg_ui_system::max_index = UINT8_MAX;
          npc_dlg_ui_system::npc_id = npc_id;
          npc_dlg_ui_system::time = window::dt_now;
          // string.wz找d0
          npc_dlg_ui_system::text =
              npc_game_instance::load_npc_text(npc_id, u"d0");
          npc_dlg_ui_system::cb = {};
          r = true;
          break;
        }
        default: {
          break;
        }
        }
      }
    }
  }
  default: {
    break;
  }
  }
  return r;
}

bool cursor_logic_system::event(SDL_Event *event) {
  if (event_cursor_hand(event)) {
    return false;
  }
  if (event_character(event)) {
    return false;
  }
  if (event_npc(event)) {
    return false;
  }

  return true;
}