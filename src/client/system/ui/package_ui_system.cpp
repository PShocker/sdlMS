#include "package_ui_system.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_scancode.h"
#include "scroll_ui_system.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/system/system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "tooltip_ui_system.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <string>
#include <utility>
#include <vector>

std::vector<uint32_t> package_ui_system::load_blank_index(uint32_t tab) {
  std::vector<uint32_t> r;
  if (tab == 0) {
    auto &equips = package_game_instance::equips;
    for (int32_t i = 0; i < equips.size(); i++) {
      if (!equips[i].has_value()) {
        r.push_back(i);
      }
    }
  } else {
    std::vector<std::optional<game_item>> *items;
    switch (tab) {
    case 1: {
      items = &package_game_instance::cosumes;
      break;
    }
    case 2: {
      items = &package_game_instance::etc;
      break;
    }
    case 3: {
      items = &package_game_instance::install;
      break;
    }
    case 4: {
      items = &package_game_instance::cash;
      break;
    }
    }
    for (int32_t i = 0; i < items->size(); i++) {
      if (!items->at(i).has_value()) {
        r.push_back(i);
      }
    }
  }
  return r;
}

std::optional<uint32_t> package_ui_system::load_mouse_index() {
  SDL_FPoint slot_pos{8, 51};
  const auto slot_space_x = 4;
  const auto slot_space_y = 2;

  auto &mouse_pos = window::mouse_pos;
  auto lx = pos.x + slot_pos.x;
  auto rx = lx + 36 * 5;

  auto ty = pos.y + slot_pos.y;
  auto by = ty + 34 * 6;
  if ((lx <= mouse_pos.x && mouse_pos.x <= rx) &&
      (ty <= mouse_pos.y && mouse_pos.y <= by)) {
    auto dx = int((mouse_pos.x - lx) / 36);
    auto dy = int((mouse_pos.y - ty) / 34);
    auto index = dy * 5 + dx;
    index += page;
    return index;
  }

  return std::nullopt;
}

void package_ui_system::render_backgrnd() {
  static auto texture =
      wz_resource::load_texture(wz_resource::ui->find(u"Item.img/backgrnd"));
  SDL_FRect pos_rect{pos.x, pos.y, static_cast<float>(texture->w),
                     static_cast<float>(texture->h)};
  SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
}

void package_ui_system::render_tab() {
  const static std::array tab_pos = {
      SDL_FPoint{5, 24},   //
      SDL_FPoint{38, 24},  //
      SDL_FPoint{71, 24},  //
      SDL_FPoint{104, 24}, //
      SDL_FPoint{137, 24}, //
      SDL_FPoint{170, 24}, //
  };
  const static auto tab_node = wz_resource::ui->find(u"Item.img/Tab");
  const static std::array active_texture = {
      wz_resource::load_texture(tab_node->find(u"enabled/0")),
      wz_resource::load_texture(tab_node->find(u"enabled/1")),
      wz_resource::load_texture(tab_node->find(u"enabled/2")),
      wz_resource::load_texture(tab_node->find(u"enabled/3")),
      wz_resource::load_texture(tab_node->find(u"enabled/4")),
      wz_resource::load_texture(tab_node->find(u"enabled/5")),
  };
  const static std::array disabled_texture = {
      wz_resource::load_texture(tab_node->find(u"disabled/0")),
      wz_resource::load_texture(tab_node->find(u"disabled/1")),
      wz_resource::load_texture(tab_node->find(u"disabled/2")),
      wz_resource::load_texture(tab_node->find(u"disabled/3")),
      wz_resource::load_texture(tab_node->find(u"disabled/4")),
      wz_resource::load_texture(tab_node->find(u"disabled/5")),
  };
  for (uint8_t i = 0; i < tab_pos.size(); i++) {
    SDL_Texture *t = active_tab == i ? active_texture[i] : disabled_texture[i];
    SDL_FRect pos_rect{static_cast<float>(int(pos.x + tab_pos[i].x)),
                       static_cast<float>(int(pos.y + tab_pos[i].y)),
                       static_cast<float>(t->w), static_cast<float>(t->h)};
    SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
  }
}

void package_ui_system::render_items_info() {
  auto index = load_mouse_index();
  if (index.has_value()) {
    if (active_tab == 0) {
      auto &equips = package_game_instance::equips;
      auto equip = equips.at(index.value());
      if (equip.has_value()) {
        auto &mouse_pos = window::mouse_pos;
        SDL_FPoint show_pos = {mouse_pos.x + 15, mouse_pos.y + 15};
        tooltip_ui_system::render_equip(equip.value(), show_pos.x, show_pos.y);
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
      if (r->at(index.value()).has_value()) {
        auto &mouse_pos = window::mouse_pos;
        SDL_FPoint show_pos = {mouse_pos.x + 15, mouse_pos.y + 15};
        tooltip_ui_system::render_item(r->at(index.value()).value(), show_pos.x,
                                       show_pos.y);
      }
    }
  }
}

void package_ui_system::render_items() {
  SDL_FPoint slot_pos{8, 51};
  const auto slot_space_x = 4;
  const auto slot_space_y = 2;
  if (active_tab == 0) {
    auto &equips = package_game_instance::equips;
    for (uint8_t i = page * 5; i <= equips.size(); i++) {
      auto row = i / 5 - page;
      auto col = i % 5;

      if (row >= 6) {
        break;
      }
      if (!equips[i].has_value()) {
        continue;
      }

      auto id = equips[i]->id;
      auto info = equip_game_instance::load_equip_info(id);
      auto icon = wz_resource::load_texture(info->get_child(u"icon"));

      auto x = pos.x + slot_pos.x + col * 32 + col * slot_space_x +
               (32 - icon->w) / 2;
      auto y =
          pos.y + slot_pos.y + row * 32 + row * slot_space_y + 32 - icon->h;
      SDL_FRect pos_rect{
          static_cast<float>(int(x)),
          static_cast<float>(int(y)),
          static_cast<float>(icon->w),
          static_cast<float>(icon->h),
      };
      SDL_RenderTexture(window::renderer, icon, nullptr, &pos_rect);
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
    for (uint8_t i = page * 5; i <= r->size(); i++) {
      auto row = i / 5 - page;
      auto col = i % 5;

      if (row >= 6) {
        break;
      }
      if (!r->at(i).has_value()) {
        continue;
      }

      auto id = r->at(i)->id;
      auto info = item_game_instance::load_item_info(id);
      auto icon = wz_resource::load_texture(info->get_child(u"icon"));
      auto x = pos.x + slot_pos.x + col * 32 + col * slot_space_x +
               (32 - icon->w) / 2;
      auto y =
          pos.y + slot_pos.y + row * 32 + row * slot_space_y + 32 - icon->h;
      SDL_FRect pos_rect{
          static_cast<float>(int(x)),
          static_cast<float>(int(y)),
          static_cast<float>(icon->w),
          static_cast<float>(icon->h),
      };
      SDL_RenderTexture(window::renderer, icon, nullptr, &pos_rect);
    }
  }
}

void package_ui_system::render_scroll() {
  const SDL_FPoint lt{188, 51};
  const uint32_t length = 202;
  auto size = 96 / 5;
  auto cursor_in = cursor_game_instance::cursor_ui;
  bool top = cursor_in == render;
  scroll_ui_system::render_vscroll((int)pos.x + lt.x, (int)pos.y + lt.y, page,
                                   size, length, top);
  return;
}

void package_ui_system::render_button() {
  const static std::array buttons_nodes = {
      wz_resource::ui->find(u"Basic.img/BtClose"),
  };
  auto wh = load_wh();
  std::array buttons_rect = {
      SDL_FRect{wh.x - 20, 7, 12, 12}, //
  };

  for (size_t i = 0; i < buttons_nodes.size(); ++i) {
    auto k = buttons_nodes[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    auto &mouse_pos = window::mouse_pos;
    // 判断按钮是否被遮挡
    auto cursor_in = cursor_game_instance::cursor_ui;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && cursor_in == render) {
      if (window::mouse_state & SDL_BUTTON_LMASK) {
        auto pressed = wz_resource::load_texture(k->find(u"pressed/0"));
        SDL_RenderTexture(window::renderer, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = wz_resource::load_texture(k->find(u"mouseOver/0"));
        SDL_RenderTexture(window::renderer, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = wz_resource::load_texture(k->find(u"normal/0"));
      SDL_RenderTexture(window::renderer, normal, nullptr, &pos_rect);
    }
  }
}

bool package_ui_system::render() {
  render_backgrnd();
  render_tab();
  render_items();
  render_scroll();
  render_button();
  render_items_info();
  return true;
}

SDL_FPoint package_ui_system::load_wh() { return {209, 289}; }

void package_ui_system::open() {
  auto wh = load_wh();
  auto &camera = camera_game_instance::camera;
  pos.x = (camera.w - wh.x) / 2;
  pos.y = (camera.h - wh.y) / 2;

  system::render_systems.insert(system::render_systems.end() - 1, render);
  system::event_systems.insert(system::event_systems.begin(), event);
}

void package_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
}

void package_ui_system::event_top() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
  std::erase(system::logic_systems, run);

  system::render_systems.insert(system::render_systems.end() - 1, render);
  system::event_systems.insert(system::event_systems.begin(), event);
  system::logic_systems.push_back(run);
}

bool package_ui_system::event_click_item(SDL_Event *event) {
  if (cursor_game_instance::cursor_hand.has_value()) {
    auto hand = cursor_game_instance::cursor_hand.value();
    if (hand.type == cursor_game_instance::package && hand.val == active_tab) {
      auto index = load_mouse_index();
      if (active_tab == 0) {
        auto equips = package_game_instance::equips;
        if (index.has_value()) {
          if (hand.val2 == index.value()) {
            auto &self = character_game_instance::self;
            auto equip = equips[index.value()].value();
            auto ev = equip_game_instance::load_equip_slot(equip, self);
            auto blank_slot = load_blank_index(active_tab);
            blank_slot.push_back(index.value());
            std::ranges::sort(blank_slot);
            if (blank_slot.size() >= ev.size()) {
              equips[hand.val2] = std::nullopt;
              for (int32_t i = 0; i < ev.size(); i++) {
                equips[blank_slot[i]] = ev[i];
              }
              equip_game_instance::add_equip(equip, self, -1);
              // 发包
              ClientCharacterT ct;
              ct.map_id = scene_system_instance::map_id;
              auto c = character_game_instance::load_characterT(self);
              ct.payload = std::make_unique<fbs::CharacterT>(std::move(c));
              client_request::send_to_host(ct);
            } else {
              // 无空闲位置
            }
          } else {
            std::swap(equips[hand.val2], equips[index.value()]);
          }
          package_game_instance::equips = equips;
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
        if (index.has_value()) {
          if (hand.val2 == index.value()) {
          } else {
            std::swap(r->at(hand.val2), r->at(index.value()));
          }
        }
      }
      cursor_game_instance::cursor_hand = std::nullopt;
    }
  } else {
    auto index = load_mouse_index();
    if (!index.has_value()) {
      return false;
    }
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
    cursor_game_instance::cursor_hand = cursor_game_instance::cursor_hand_data{
        .type = cursor_game_instance::package,
        .val = active_tab,
        .val2 = index.value(),
    };
  }
  return true;
}

void package_ui_system::event_drag_start(SDL_Event *event) {
  auto wh = load_wh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - event->button.x, pos.y - event->button.y};
  }
  return;
}

void package_ui_system::event_drag_end() {
  drag = std::nullopt;
  return;
}

void package_ui_system::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    auto &camera = camera_game_instance::camera;
    auto [w, h] = load_wh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}

void package_ui_system::toggle() {
  auto fn = &render;
  if (std::ranges::contains(system::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

bool package_ui_system::cursor_in() {
  auto [w, h] = load_wh();
  auto &mouse = window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

void package_ui_system::event_tab(SDL_Event *event) {
  const static std::array tab_rect = {
      SDL_FRect{5, 24, 33, 19},   //
      SDL_FRect{38, 24, 33, 19},  //
      SDL_FRect{71, 24, 33, 19},  //
      SDL_FRect{104, 24, 33, 19}, //
      SDL_FRect{137, 24, 33, 19}, //
      SDL_FRect{170, 24, 33, 19}, //
  };
  for (uint8_t i = 0; i < tab_rect.size(); i++) {
    auto pos_rect = tab_rect[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      active_tab = i;
    }
  }
}

void package_ui_system::event_close() { close(); }

bool package_ui_system::event_button(SDL_Event *event) {
  std::vector<SDL_FRect> r;
  std::vector<void (*)()> fns;
  auto wh = load_wh();
  r = {
      SDL_FRect{wh.x - 20, 7, 12, 12}, //
  };
  fns = {
      event_close,
  };

  for (size_t i = 0; i < r.size(); ++i) {
    auto pos_rect = r[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      fns[i]();
      return true;
    }
  }

  return false;
}

bool package_ui_system::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_KEY_DOWN: {
    auto scan_code = event->key.scancode;
    switch (scan_code) {
    case SDL_SCANCODE_ESCAPE: {
      event_close();
      return false;
      break;
    }
    default: {
      break;
    }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (cursor_game_instance::cursor_ui == render) {
        event_top();
        event_drag_start(event);
        r = false;
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (cursor_game_instance::cursor_ui == render) {
        event_tab(event);
        event_click_item(event);
        r = event_button(event);
      }
      event_drag_end();
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    event_drag_move(event);
    break;
  }
  default: {
    break;
  }
  }

  return r;
}

bool package_ui_system::run() { return true; }