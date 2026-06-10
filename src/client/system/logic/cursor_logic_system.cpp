#include "cursor_logic_system.h"
#include "character_logic_system.h"
#include "npc_logic_system.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/character_info_ui_system.h"
#include "src/client/system/ui/character_stat_ui_system.h"
#include "src/client/system/ui/craft_ui_system.h"
#include "src/client/system/ui/equip_ui_system.h"
#include "src/client/system/ui/minimap_ui_system.h"
#include "src/client/system/ui/package_ui_system.h"
#include "src/client/system/ui/revive_ui_system.h"
#include "src/client/system/ui/skill_ui_system.h"
#include "src/client/system/ui/statusbar_ui_system.h"
#include "src/client/system/ui/worldmap_ui_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/physic/physic.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include "wz/Wz.h"
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <optional>
#include <string>
#include <utility>

using namespace fbs;

bool cursor_logic_system::run_package_motion() {
  auto index = package_ui_system::load_mouse_index();
  if (!index.has_value()) {
    return false;
  }
  auto active_tab = package_ui_system::active_tab;
  if (active_tab == 0) {
    if (!package_game_instance::equips[index.value()].has_value()) {
      return false;
    }
  } else {
    std::vector<std::optional<game_item>> *r;
    switch (active_tab) {
    case 1: {
      r = &package_game_instance::cosumes;
      break;
    }
    case 2: {
      r = &package_game_instance::etc;
      break;
    }
    case 3: {
      r = &package_game_instance::install;
      break;
    }
    case 4: {
      r = &package_game_instance::cash;
      break;
    }
    default: {
      break;
    }
    }
    if (!r->at(index.value()).has_value()) {
      return false;
    }
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
              auto equip = package_game_instance::equips[cursor_hand->val2];
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

              cursor_game_instance::cursor_hand_drop_id = dt.random_id;

              // package_game_instance::equips[cursor_hand->val2] =
              // std::nullopt;
            } else {
              std::vector<std::optional<game_item>> *r;
              switch (active_tab) {
              case 1: {
                r = &package_game_instance::cosumes;
                break;
              }
              case 2: {
                r = &package_game_instance::etc;
                break;
              }
              case 3: {
                r = &package_game_instance::install;
                break;
              }
              case 4: {
                r = &package_game_instance::cash;
                break;
              }
              default: {
                break;
              }
              }
            }
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

bool cursor_logic_system::event_character_info(SDL_Event *event) {
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
          auto &ui_character = character_info_ui_system::character;
          ui_character = character.g_character;
          ui_character.flip = 0;
          character_logic_system::run_stand_action(ui_character);
          character_logic_system::run_face_action(ui_character, u"default");
          character_info_ui_system::close();
          character_info_ui_system::open();
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
      auto npc = npc_logic_system::cursor_in();
      if (npc.has_value()) {
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
  if (event_character_info(event)) {
    return false;
  }
  if (event_npc(event)) {
    return false;
  }

  return true;
}