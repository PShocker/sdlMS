#include "equip_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "tooltip_ui_system.h"
#include <algorithm>
#include <string>

SDL_FPoint equip_ui_system::load_wh() { return {175, 289}; }

void equip_ui_system::render_backgrnd() {
  static auto backgrnd = wz_resource::load_texture(
      wz_resource::ui->find(u"Equipment.img/backgrnd"));

  SDL_FRect pos_rect{pos.x, pos.y, static_cast<float>(backgrnd->w),
                     static_cast<float>(backgrnd->h)};
  SDL_RenderTexture(window::renderer, backgrnd, nullptr, &pos_rect);
}

void equip_ui_system::render_equip_texture(std::u16string &id,
                                           SDL_FPoint slot) {
  const SDL_FPoint lt{4, 45};

  auto info = equip_game_instance::load_equip_info(id);
  auto icon = wz_resource::load_texture(info->get_child(u"icon"));
  auto x = (int)pos.x + slot.x + lt.x + (32 - icon->w) / 2;
  auto y = (int)pos.y + slot.y + lt.y + (32 - icon->h) / 2;
  SDL_FRect pos_rect{
      x,
      y,
      static_cast<float>(icon->w),
      static_cast<float>(icon->h),
  };
  SDL_RenderTexture(window::renderer, icon, nullptr, &pos_rect);
}

const static SDL_FPoint cap_slot{68, 23};
const static SDL_FPoint earacc_slot{101, 56};
const static SDL_FPoint clothes_slot{35, 89};
const static SDL_FPoint pants_slot{35, 122};
const static SDL_FPoint shoes_slot{35, 155};
const static SDL_FPoint gloves_slot{2, 122};
const static SDL_FPoint cape_slot{2, 89};
const static SDL_FPoint shield_slot{134, 89};
const static SDL_FPoint weapon_slot{101, 89};

void equip_ui_system::render_equip() {
  const auto &self = character_game_instance::self;
  if (self.cap.has_value()) {
    auto id = self.cap->id;
    render_equip_texture(id, cap_slot);
  }
  if (self.accessory.has_value()) {
    auto id = self.accessory->id;
    render_equip_texture(id, earacc_slot);
  }
  if (self.coat.has_value()) {
    auto id = self.coat->id;
    render_equip_texture(id, clothes_slot);
  }
  if (self.pant.has_value()) {
    auto id = self.pant->id;
    render_equip_texture(id, pants_slot);
  }
  if (self.shoes.has_value()) {
    auto id = self.shoes->id;
    render_equip_texture(id, pants_slot);
  }
  if (self.glove.has_value()) {
    auto id = self.glove->id;
    render_equip_texture(id, gloves_slot);
  }
  if (self.cape.has_value()) {
    auto id = self.cape->id;
    render_equip_texture(id, cape_slot);
  }
  if (self.shield.has_value()) {
    auto id = self.shield->id;
    render_equip_texture(id, shield_slot);
  }
  if (self.weapon.has_value()) {
    auto id = self.weapon->id;
    render_equip_texture(id, weapon_slot);
  }
}

void equip_ui_system::render_equip_info() {
  const SDL_FPoint lt{4, 45};
  auto &self = character_game_instance::self;
  SDL_FRect pos_rect{
      pos.x + cap_slot.x + lt.x,
      pos.y + cap_slot.y + lt.y,
      32,
      32,
  };
  auto &mouse_pos = window::mouse_pos;
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    if (self.cap.has_value()) {
      tooltip_ui_system::render_equip(self.cap.value(), mouse_pos.x,
                                      mouse_pos.y);
    }
    return;
  }
  pos_rect = {
      pos.x + earacc_slot.x + lt.x,
      pos.y + earacc_slot.y + lt.y,
      32,
      32,
  };
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    if (self.accessory.has_value()) {
      tooltip_ui_system::render_equip(self.accessory.value(), mouse_pos.x,
                                      mouse_pos.y);
    }
    return;
  }
  pos_rect = {
      pos.x + clothes_slot.x + lt.x,
      pos.y + clothes_slot.y + lt.y,
      32,
      32,
  };
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    if (self.coat.has_value()) {
      tooltip_ui_system::render_equip(self.coat.value(), mouse_pos.x,
                                      mouse_pos.y);
    }
    return;
  }
  pos_rect = {
      pos.x + pants_slot.x + lt.x,
      pos.y + pants_slot.y + lt.y,
      32,
      32,
  };
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    if (self.pant.has_value()) {
      tooltip_ui_system::render_equip(self.pant.value(), mouse_pos.x,
                                      mouse_pos.y);
    }
    return;
  }
  pos_rect = {
      pos.x + shoes_slot.x + lt.x,
      pos.y + shoes_slot.y + lt.y,
      32,
      32,
  };
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    if (self.shoes.has_value()) {
      tooltip_ui_system::render_equip(self.shoes.value(), mouse_pos.x,
                                      mouse_pos.y);
    }
    return;
  }
  pos_rect = {
      pos.x + gloves_slot.x + lt.x,
      pos.y + gloves_slot.y + lt.y,
      32,
      32,
  };
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    if (self.glove.has_value()) {
      tooltip_ui_system::render_equip(self.glove.value(), mouse_pos.x,
                                      mouse_pos.y);
    }
    return;
  }
  pos_rect = {
      pos.x + cape_slot.x + lt.x,
      pos.y + cape_slot.y + lt.y,
      32,
      32,
  };
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    if (self.cape.has_value()) {
      tooltip_ui_system::render_equip(self.cape.value(), mouse_pos.x,
                                      mouse_pos.y);
    }
    return;
  }
  pos_rect = {
      pos.x + shield_slot.x + lt.x,
      pos.y + shield_slot.y + lt.y,
      32,
      32,
  };
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    if (self.shield.has_value()) {
      tooltip_ui_system::render_equip(self.shield.value(), mouse_pos.x,
                                      mouse_pos.y);
    }
    return;
  }
  pos_rect = {
      pos.x + weapon_slot.x + lt.x,
      pos.y + weapon_slot.y + lt.y,
      32,
      32,
  };
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    if (self.weapon.has_value()) {
      tooltip_ui_system::render_equip(self.weapon.value(), mouse_pos.x,
                                      mouse_pos.y);
    }
    return;
  }
}

void equip_ui_system::render_tab() {
  static auto node = wz_resource::ui->find(u"Equipment.img/tab:mainTab");
  const SDL_FPoint lt{5, 26};
  const SDL_FPoint rb{167, 42};
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
    static auto t = wz_resource::ui->find(u"Equipment.img/equip/backgrnd");
    static auto texture = wz_resource::load_texture(t);
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

bool equip_ui_system::render() {
  render_backgrnd();
  render_backgrnd2();
  render_tab();
  render_button();
  render_equip();
  render_equip_info();
  return true;
}

void equip_ui_system::open() {
  auto wh = load_wh();
  auto &camera = camera_game_instance::camera;
  pos.x = (camera.w - wh.x) / 2;
  pos.y = (camera.h - wh.y) / 2;

  system::render_systems.insert(system::render_systems.end() - 1, render);
  system::event_systems.insert(system::event_systems.end() - 1, event);
}

void equip_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
}

void equip_ui_system::toggle() {
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

  system::render_systems.insert(system::render_systems.end() - 1, render);
  system::event_systems.insert(system::event_systems.end() - 1, event);
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

bool equip_ui_system::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
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
        r = !event_button(event);
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