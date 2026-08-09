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
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "tooltip_ui_system.h"
#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <utility>

const static SDL_FPoint cap_slot{68, 23};
const static SDL_FPoint earacc_slot{101, 56};
const static SDL_FPoint clothes_slot{35, 89};
const static SDL_FPoint pants_slot{35, 122};
const static SDL_FPoint shoes_slot{35, 155};
const static SDL_FPoint gloves_slot{2, 122};
const static SDL_FPoint cape_slot{2, 89};
const static SDL_FPoint shield_slot{134, 89};
const static SDL_FPoint weapon_slot{101, 89};

std::optional<equip_ui_system::equip_mouse_index>
equip_ui_system::load_mouse_index() {
  auto cursor_in = cursor_game_instance::cursor_ui;
  if (cursor_in != render) {
    return std::nullopt;
  }
  auto &mouse_pos = window::mouse_pos;
  const SDL_FPoint lt{4, 45};
  auto x = (int)pos.x + cap_slot.x + lt.x;
  auto y = (int)pos.y + cap_slot.y + lt.y;
  SDL_FRect pos_rect{
      x,
      y,
      static_cast<float>(32),
      static_cast<float>(32),
  };
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    return cap;
  }
  pos_rect.x = (int)pos.x + earacc_slot.x + lt.x;
  pos_rect.y = (int)pos.y + earacc_slot.y + lt.y;
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    return earcc;
  }
  pos_rect.x = (int)pos.x + clothes_slot.x + lt.x;
  pos_rect.y = (int)pos.y + clothes_slot.y + lt.y;
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    return clothes;
  }
  pos_rect.x = (int)pos.x + pants_slot.x + lt.x;
  pos_rect.y = (int)pos.y + pants_slot.y + lt.y;
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    return pants;
  }
  pos_rect.x = (int)pos.x + shoes_slot.x + lt.x;
  pos_rect.y = (int)pos.y + shoes_slot.y + lt.y;
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    return shoes;
  }
  pos_rect.x = (int)pos.x + gloves_slot.x + lt.x;
  pos_rect.y = (int)pos.y + gloves_slot.y + lt.y;
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    return gloves;
  }
  pos_rect.x = (int)pos.x + cape_slot.x + lt.x;
  pos_rect.y = (int)pos.y + cape_slot.y + lt.y;
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    return cape;
  }
  pos_rect.x = (int)pos.x + shield_slot.x + lt.x;
  pos_rect.y = (int)pos.y + shield_slot.y + lt.y;
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    return shield;
  }
  pos_rect.x = (int)pos.x + weapon_slot.x + lt.x;
  pos_rect.y = (int)pos.y + weapon_slot.y + lt.y;
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    return weapon;
  }
  return std::nullopt;
}

std::optional<game_equip_item> *
equip_ui_system::load_equip(equip_mouse_index index) {
  std::optional<game_equip_item> *equip;
  auto &self = character_game_instance::self;
  switch (index) {
  case cap: {
    equip = &self.cap;
    break;
  }
  case earcc: {
    equip = &self.accessory;
    break;
  }
  case clothes: {
    equip = &self.coat;
    break;
  }
  case pants: {
    equip = &self.pant;
    break;
  }
  case shoes: {
    equip = &self.shoes;
    break;
  }
  case gloves: {
    equip = &self.glove;
    break;
  }
  case cape: {
    equip = &self.cape;
    break;
  }
  case shield: {
    equip = &self.shield;
    break;
  }
  case weapon: {
    equip = &self.weapon;
    break;
  }
  case ring0:
  case ring1:
  case ring2:
  case ring3:
    break;
  }
  return equip;
}

std::optional<game_deco_item> *
equip_ui_system::load_deco(equip_mouse_index index) {
  std::optional<game_deco_item> *deco;
  auto &self = character_game_instance::self;
  switch (index) {
  case cap: {
    deco = &self.cap_deco;
    break;
  }
  case earcc: {
    deco = &self.accessory_deco;
    break;
  }
  case clothes: {
    deco = &self.coat_deco;
    break;
  }
  case pants: {
    deco = &self.pant_deco;
    break;
  }
  case shoes: {
    deco = &self.shoes_deco;
    break;
  }
  case gloves: {
    deco = &self.glove_deco;
    break;
  }
  case cape: {
    deco = &self.cape_deco;
    break;
  }
  case shield: {
    deco = &self.shield_deco;
    break;
  }
  case weapon: {
    deco = &self.weapon_deco;
    break;
  }
  case ring0:
  case ring1:
  case ring2:
  case ring3:
    break;
  }
  return deco;
}

SDL_FPoint equip_ui_system::load_wh() { return {175, 289}; }

void equip_ui_system::render_backgrnd() {
  static auto backgrnd = wz_resource::load_texture(
      wz_resource::ui->find(u"Equipment.img/backgrnd"));

  SDL_FRect pos_rect{pos.x, pos.y, static_cast<float>(backgrnd->w),
                     static_cast<float>(backgrnd->h)};
  SDL_RenderTexture(window::renderer, backgrnd, nullptr, &pos_rect);
}

void equip_ui_system::render_equip_texture(game_equip_item &equip,
                                           SDL_FPoint slot) {
  const SDL_FPoint lt{4, 45};

  auto info = equip_game_instance::load_equip_info(equip.id);
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

void equip_ui_system::render_equip() {
  auto &self = character_game_instance::self;
  if (self.cap.has_value()) {
    render_equip_texture(self.cap.value(), cap_slot);
  }
  if (self.accessory.has_value()) {
    render_equip_texture(self.accessory.value(), earacc_slot);
  }
  if (self.coat.has_value()) {
    render_equip_texture(self.coat.value(), clothes_slot);
  }
  if (self.pant.has_value()) {
    render_equip_texture(self.pant.value(), pants_slot);
  }
  if (self.shoes.has_value()) {
    render_equip_texture(self.shoes.value(), pants_slot);
  }
  if (self.glove.has_value()) {
    render_equip_texture(self.glove.value(), gloves_slot);
  }
  if (self.cape.has_value()) {
    render_equip_texture(self.cape.value(), cape_slot);
  }
  if (self.shield.has_value()) {
    render_equip_texture(self.shield.value(), shield_slot);
  }
  if (self.weapon.has_value()) {
    render_equip_texture(self.weapon.value(), weapon_slot);
  }
}

void equip_ui_system::render_deco_texture(game_deco_item &deco,
                                          SDL_FPoint slot) {
  const SDL_FPoint lt{4, 45};
  auto info = equip_game_instance::load_equip_info(deco.id);
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
  icon = wz_resource::load_texture(
      wz_resource::ui->find(u"CashShop.img/CashItem/0"));
  pos_rect.x = (int)pos.x + slot.x + lt.x + 19;
  pos_rect.y = (int)pos.y + slot.y + lt.y + 19;
  pos_rect.w = icon->w;
  pos_rect.h = icon->h;
  SDL_RenderTexture(window::renderer, icon, nullptr, &pos_rect);
}

void equip_ui_system::render_deco() {
  auto &self = character_game_instance::self;
  if (self.cap_deco.has_value()) {
    render_deco_texture(self.cap_deco.value(), cap_slot);
  }
  if (self.accessory_deco.has_value()) {
    render_deco_texture(self.accessory_deco.value(), earacc_slot);
  }
  if (self.coat_deco.has_value()) {
    render_deco_texture(self.coat_deco.value(), clothes_slot);
  }
  if (self.pant_deco.has_value()) {
    render_deco_texture(self.pant_deco.value(), pants_slot);
  }
  if (self.shoes_deco.has_value()) {
    render_deco_texture(self.shoes_deco.value(), pants_slot);
  }
  if (self.glove_deco.has_value()) {
    render_deco_texture(self.glove_deco.value(), gloves_slot);
  }
  if (self.cape_deco.has_value()) {
    render_deco_texture(self.cape_deco.value(), cape_slot);
  }
  if (self.shield_deco.has_value()) {
    render_deco_texture(self.shield_deco.value(), shield_slot);
  }
  if (self.weapon_deco.has_value()) {
    render_deco_texture(self.weapon_deco.value(), weapon_slot);
  }
}

void equip_ui_system::render_equip_info() {
  auto index = load_mouse_index();
  if (index.has_value() && !cursor_game_instance::modal_overlay) {
    auto &mouse_pos = window::mouse_pos;
    SDL_FPoint show_pos = {mouse_pos.x + 15, mouse_pos.y + 15};
    auto equip = load_equip(index.value());
    if (equip->has_value()) {
      tooltip_ui_system::render_equip(equip->value(), show_pos.x, show_pos.y);
    }
  }
}

void equip_ui_system::render_deco_info() {
  auto index = load_mouse_index();
  if (index.has_value() && !cursor_game_instance::modal_overlay) {
    auto &mouse_pos = window::mouse_pos;
    SDL_FPoint show_pos = {mouse_pos.x + 15, mouse_pos.y + 15};
    auto deco = load_deco(index.value());
    if (deco->has_value()) {
      tooltip_ui_system::render_deco(deco->value(), show_pos.x, show_pos.y);
    }
  }
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
    render_equip_info();
  } else {
    render_deco();
    render_deco_info();
  }

  return true;
}

bool equip_ui_system::event_click_equip(SDL_Event *event) {
  if (cursor_game_instance::cursor_hand_net.has_value()) {
    return false;
  }
  auto index = load_mouse_index();
  if (!index.has_value()) {
    return false;
  }
  auto eqp = *load_equip(index.value());
  auto &self = character_game_instance::self;
  auto cursor_hand = cursor_game_instance::cursor_hand;
  if (cursor_hand.has_value()) {
    switch (cursor_hand->type) {
    case cursor_game_instance::equipment: {
      break;
    }
    case cursor_game_instance::package: {
      auto &equip = package_game_instance::data[0][cursor_hand->sub_val];
      auto ep = static_cast<game_equip_item &>(*equip);
      if (equip_game_instance::add_equip_limit(ep, self, index.value())) {
      } else {
        // dialog
      }
      break;
    }
    default: {
      break;
    }
    }
  } else {
    if (event->button.button == SDL_BUTTON_LEFT) {
      cursor_game_instance::cursor_hand = {
          .type = cursor_game_instance::equipment,
          .val = 0,
          .sub_val = index.value(),
      };
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
  }
}

void equip_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
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