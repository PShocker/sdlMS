#include "equip_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game/game_item.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "tooltip_ui_system.h"
#include <algorithm>
#include <memory>
#include <optional>
#include <string>

const static SDL_FPoint cap_slot{68, 23};
const static SDL_FPoint earacc_slot{101, 56};
const static SDL_FPoint clothes_slot{35, 89};
const static SDL_FPoint pants_slot{35, 122};
const static SDL_FPoint shoes_slot{35, 155};
const static SDL_FPoint gloves_slot{2, 122};
const static SDL_FPoint cape_slot{2, 89};
const static SDL_FPoint shield_slot{134, 89};
const static SDL_FPoint weapon_slot{101, 89};

SDL_FPoint equip_ui_system::load_wh() { return {175, 289}; }

void equip_ui_system::render_backgrnd() {
  static auto backgrnd = wz_resource::load_texture(
      wz_resource::ui->find(u"Equipment.img/backgrnd"));

  SDL_FRect pos_rect{pos.x, pos.y, static_cast<float>(backgrnd->w),
                     static_cast<float>(backgrnd->h)};
  SDL_RenderTexture(window::renderer, backgrnd, nullptr, &pos_rect);
}

void equip_ui_system::render_equip_texture(
    std::optional<game_equip_item> &equip, SDL_FPoint slot,
    equip_mouse_index i) {
  const SDL_FPoint lt{4, 45};
  SDL_FRect pos_rect;
  pos_rect.x = (int)pos.x + slot.x + lt.x;
  pos_rect.y = (int)pos.y + slot.y + lt.y;
  pos_rect.w = 32;
  pos_rect.h = 32;
  const auto &mouse_pos = window::mouse_pos;
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    mouse_index = i;
  }
  if (!equip.has_value()) {
    return;
  }
  auto info = equip_game_instance::load_equip_info(equip->id);
  auto icon = wz_resource::load_texture(info->get_child(u"icon"));
  auto x = (int)pos.x + slot.x + lt.x + (32 - icon->w) / 2;
  auto y = (int)pos.y + slot.y + lt.y + (32 - icon->h) / 2;
  pos_rect = {
      x,
      y,
      static_cast<float>(icon->w),
      static_cast<float>(icon->h),
  };
  SDL_RenderTexture(window::renderer, icon, nullptr, &pos_rect);
}

void equip_ui_system::render_equip() {
  mouse_index = std::nullopt;
  auto &self = character_game_instance::self;
  render_equip_texture(self.cap, cap_slot, equip_mouse_index::cap);
  render_equip_texture(self.accessory, earacc_slot, equip_mouse_index::earcc);
  render_equip_texture(self.coat, clothes_slot, equip_mouse_index::clothes);
  render_equip_texture(self.pant, pants_slot, equip_mouse_index::pants);
  render_equip_texture(self.shoes, shoes_slot, equip_mouse_index::shoes);
  render_equip_texture(self.glove, gloves_slot, equip_mouse_index::gloves);
  render_equip_texture(self.cape, cape_slot, equip_mouse_index::cape);
  render_equip_texture(self.shield, shield_slot, equip_mouse_index::shield);
  render_equip_texture(self.weapon, weapon_slot, equip_mouse_index::weapon);
}

void equip_ui_system::render_disable_texture(SDL_FPoint slot) {
  const SDL_FPoint lt{4, 45};
  static auto icon = wz_resource::load_texture(
      wz_resource::ui->find(u"Equipment.img/equip/canvas:disabled0"));
  auto x = (int)pos.x + slot.x + lt.x;
  auto y = (int)pos.y + slot.y + lt.y;
  SDL_FRect pos_rect{
      x,
      y,
      static_cast<float>(icon->w),
      static_cast<float>(icon->h),
  };
  SDL_RenderTexture(window::renderer, icon, nullptr, &pos_rect);
}

void equip_ui_system::render_deco_texture(std::optional<game_deco_item> &deco,
                                          SDL_FPoint slot,
                                          equip_mouse_index i) {
  const SDL_FPoint lt{4, 45};
  SDL_FRect pos_rect;
  pos_rect.x = (int)pos.x + slot.x + lt.x;
  pos_rect.y = (int)pos.y + slot.y + lt.y;
  pos_rect.w = 32;
  pos_rect.h = 32;
  const auto &mouse_pos = window::mouse_pos;
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    mouse_index = i;
  }
  if (!deco.has_value()) {
    return;
  }
  auto info = equip_game_instance::load_equip_info(deco->id);
  auto icon = wz_resource::load_texture(info->get_child(u"icon"));
  auto x = (int)pos.x + slot.x + lt.x + (32 - icon->w) / 2;
  auto y = (int)pos.y + slot.y + lt.y + (32 - icon->h) / 2;
  pos_rect = {
      x,
      y,
      static_cast<float>(icon->w),
      static_cast<float>(icon->h),
  };
  SDL_RenderTexture(window::renderer, icon, nullptr, &pos_rect);
  icon = wz_resource::load_texture(
      wz_resource::ms->get_root()->find(u"UI.img/CashItem"));
  pos_rect.x = (int)pos.x + slot.x + lt.x + 19;
  pos_rect.y = (int)pos.y + slot.y + lt.y + 19;
  pos_rect.w = icon->w;
  pos_rect.h = icon->h;
  SDL_RenderTexture(window::renderer, icon, nullptr, &pos_rect);
}

void equip_ui_system::render_deco() {
  mouse_index = std::nullopt;
  auto &self = character_game_instance::self;
  render_deco_texture(self.cap_deco, cap_slot, equip_mouse_index::cap);
  render_deco_texture(self.accessory_deco, earacc_slot,
                      equip_mouse_index::earcc);
  render_deco_texture(self.coat_deco, clothes_slot, equip_mouse_index::clothes);
  render_deco_texture(self.pant_deco, pants_slot, equip_mouse_index::pants);
  render_deco_texture(self.shoes_deco, shoes_slot, equip_mouse_index::shoes);
  render_deco_texture(self.glove_deco, gloves_slot, equip_mouse_index::gloves);
  render_deco_texture(self.cape_deco, cape_slot, equip_mouse_index::cape);
  render_deco_texture(self.shield_deco, shield_slot, equip_mouse_index::shield);
  render_deco_texture(self.weapon_deco, weapon_slot, equip_mouse_index::weapon);
  bool weapon_deco = false;
  if (self.weapon_deco.has_value()) {
    if (self.weapon_deco.has_value()) {
      if (self.weapon.has_value()) {
        std::u16string sub = self.weapon->id.substr(2, 2);
        std::u16string deco_val = self.weapon_deco->id + u"/" + sub;
        if (character_game_instance::avatar_data.contains(deco_val)) {
          weapon_deco = true;
        }
      }
    }
    if (!weapon_deco) {
      render_disable_texture(weapon_slot);
    }
  }
}

bool equip_ui_system::render_info() {
  auto index = mouse_index;
  if (index.has_value() && !cursor_game_instance::modal_overlay) {
    auto &mouse_pos = window::mouse_pos;
    SDL_FPoint show_pos = {mouse_pos.x + 15, mouse_pos.y + 15};
    if (active_tab == 0) {
      auto equip = equip_game_instance::load_equip((int)index.value());
      if (equip->has_value()) {
        tooltip_ui_system::render_equip(equip->value(), show_pos.x, show_pos.y);
      }
    } else {
      auto deco = equip_game_instance::load_deco((int)index.value());
      if (deco->has_value()) {
        tooltip_ui_system::render_deco(deco->value(), show_pos.x, show_pos.y);
      }
    }
  }
  return true;
}

void equip_ui_system::render_tab() {
  static auto node = wz_resource::ui->find(u"Equipment.img/tab:mainTab");
  const SDL_FPoint lt{5, 26};
  for (auto i : {0, 1}) {
    SDL_Texture *texture;
    if (active_tab == i) {
      texture = wz_resource::load_texture(
          node->get_child(u"selected")->get_child(std::to_string(i)));
    } else {
      texture = wz_resource::load_texture(
          node->get_child(u"normal")->get_child(std::to_string(i)));
    }
    SDL_FRect pos_rect = {
        int(pos.x) + lt.x + texture->w * i,
        int(pos.y) + lt.y,
        static_cast<float>(texture->w),
        static_cast<float>(texture->h),
    };
    SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
  }
}

void equip_ui_system::render_backgrnd2() {
  const SDL_FPoint lt{4, 45};
  if (active_tab == 0) {
    static auto texture = wz_resource::load_texture(
        wz_resource::ui->find(u"Equipment.img/equip/backgrnd"));
    SDL_FRect pos_rect = {
        int(pos.x) + lt.x,
        int(pos.y) + lt.y,
        static_cast<float>(texture->w),
        static_cast<float>(texture->h),
    };
    SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
  } else {
    static auto texture = wz_resource::load_texture(
        wz_resource::ui->find(u"Equipment.img/cash/backgrnd"));
    SDL_FRect pos_rect = {
        int(pos.x) + lt.x,
        int(pos.y) + lt.y,
        static_cast<float>(texture->w),
        static_cast<float>(texture->h),
    };
    SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
  }
}

void equip_ui_system::render_button() {
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
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && cursor_in == render &&
        !cursor_game_instance::modal_overlay) {
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

bool equip_ui_system::render() {
  render_backgrnd();
  render_backgrnd2();
  render_tab();
  render_button();

  if (active_tab == 0) {
    render_equip();
  } else {
    render_deco();
  }

  return true;
}

bool equip_ui_system::event_click_equip(SDL_Event *event) {
  if (cursor_game_instance::cursor_hand_net.has_value()) {
    return false;
  }
  if (!mouse_index.has_value()) {
    return false;
  }
  auto index = (int)mouse_index.value();
  auto &self = character_game_instance::self;
  auto &cursor_hand = cursor_game_instance::cursor_hand;
  if (cursor_hand.has_value()) {
    switch (cursor_hand->type) {
    case cursor_game_instance::equipment: {
      if (index == cursor_hand->sub_val && active_tab == 0) {
        equip_game_instance::unuse_equip(index);
      }
      cursor_game_instance::cursor_hand = std::nullopt;
      break;
    }
    case cursor_game_instance::package: {
      switch ((item_enum)cursor_hand->val) {
      case item_enum::equip: {
        equip_game_instance::use_equip(cursor_hand->sub_val);
        break;
      }
      case item_enum::deco: {
        equip_game_instance::use_deco(cursor_hand->sub_val);
        character_logic_system::cct.map_id = scene_system_instance::map_id;
        break;
      }
      case item_enum::consume: {
        auto &itm = package_game_instance::data[(int)item_enum::consume]
                                               [cursor_hand->sub_val];
        if (itm->id.starts_with(u"0204")) {
          if (active_tab == 0) {
            auto eqp = equip_game_instance::load_equip(index);
            if (eqp->has_value()) {
              auto &equip = eqp->value();
              auto &it = static_cast<game_consume_item &>(*itm);
              equip_game_instance::use_equip_scroll(equip, it);
              cursor_game_instance::cursor_hand = std::nullopt;
              return true;
            }
          }
        }
        break;
      }
      default: {
      }
      }
      cursor_hand = std::nullopt;
      break;
    }
    case cursor_game_instance::deco: {
      if (index == cursor_hand->sub_val && active_tab == 1) {
        equip_game_instance::unuse_deco(index);
      }
      cursor_game_instance::cursor_hand = std::nullopt;

      break;
    }
    default: {
      break;
    }
    }
  } else {
    if (event->button.button == SDL_BUTTON_LEFT) {
      bool click = false;
      if (active_tab == 0) {
        click = equip_game_instance::load_equip(index)->has_value();
      } else {
        click = equip_game_instance::load_deco(index)->has_value();
      }
      if (click) {
        if (active_tab == 0) {
          cursor_game_instance::cursor_hand = {
              .type = cursor_game_instance::equipment,
              .val = active_tab,
              .sub_val = static_cast<uint32_t>(index),
          };
        } else {
          cursor_game_instance::cursor_hand = {
              .type = cursor_game_instance::deco,
              .val = active_tab,
              .sub_val = static_cast<uint32_t>(index),
          };
        }
      }
    }
  }
  return false;
}

void equip_ui_system::open() {
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    auto wh = load_wh();
    auto &camera = camera_game_instance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);

    event_motion(nullptr);
  }
}

void equip_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::render_systems, render_info);
  std::erase(system::event_systems, event);

  event_drag_end();
}

void equip_ui_system::toggle() {
  audio_game_instance::load_audio(u"UI.img/BtMouseClick", 0);
  auto fn = &render;
  if (std::ranges::contains(system::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

bool equip_ui_system::cursor_in() {
  auto [w, h] = load_wh();
  auto &mouse = window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

void equip_ui_system::event_top() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);

    event_motion(nullptr);
  }
}

void equip_ui_system::event_drag_start(SDL_Event *event) {
  auto wh = load_wh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - event->button.x, pos.y - event->button.y};
  }
  return;
}

void equip_ui_system::event_drag_end() {
  drag = std::nullopt;
  return;
}

void equip_ui_system::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    auto &camera = camera_game_instance::camera;
    auto [w, h] = load_wh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}

void equip_ui_system::event_close() { close(); }

bool equip_ui_system::event_button(SDL_Event *event) {
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

void equip_ui_system::event_tab(SDL_Event *event) {
  const static std::array tab_rect = {
      SDL_FRect{5, 26, 33, 19},  //
      SDL_FRect{38, 26, 33, 19}, //
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

void equip_ui_system::event_motion(SDL_Event *event) {
  auto &sys = system::render_systems;
  std::erase(sys, render_info);
  auto it = std::ranges::find(sys, &cursor_render_system::render);
  if (it != sys.end()) {
    sys.insert(it, render_info);
  }
}

bool equip_ui_system::event(SDL_Event *event) {
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
        event_click_equip(event);
        r = !event_button(event);
      }
      event_drag_end();
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    event_motion(event);
    event_drag_move(event);
    break;
  }
  default: {
    break;
  }
  }

  return r;
}
