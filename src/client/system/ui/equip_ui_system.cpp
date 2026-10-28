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

SDL_FPoint EquipUiSystem::load_wh() { return {175, 289}; }

void EquipUiSystem::render_backgrnd() {
  static auto backgrnd = WzResource::load_texture(
      WzResource::ui->find(u"Equipment.img/backgrnd"));

  SDL_FRect pos_rect{pos.x, pos.y, static_cast<float>(backgrnd->w),
                     static_cast<float>(backgrnd->h)};
  SDL_RenderTexture(Window::renderer, backgrnd, nullptr, &pos_rect);
}

void EquipUiSystem::render_equip_texture(
    std::optional<GameEquipItem> &equip, SDL_FPoint slot,
    EquipMouseIndex i) {
  const SDL_FPoint lt{4, 45};
  SDL_FRect pos_rect;
  pos_rect.x = (int)pos.x + slot.x + lt.x;
  pos_rect.y = (int)pos.y + slot.y + lt.y;
  pos_rect.w = 32;
  pos_rect.h = 32;
  const auto &mouse_pos = Window::mouse_pos;
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    mouse_index = i;
  }
  if (!equip.has_value()) {
    return;
  }
  auto info = EquipGameInstance::load_equip_info(equip->id);
  auto icon = WzResource::load_texture(info->get_child(u"icon"));
  auto x = (int)pos.x + slot.x + lt.x + (32 - icon->w) / 2;
  auto y = (int)pos.y + slot.y + lt.y + (32 - icon->h) / 2;
  pos_rect = {
      x,
      y,
      static_cast<float>(icon->w),
      static_cast<float>(icon->h),
  };
  SDL_RenderTexture(Window::renderer, icon, nullptr, &pos_rect);
}

void EquipUiSystem::render_equip() {
  mouse_index = std::nullopt;
  auto &self = CharacterGameInstance::self;
  render_equip_texture(self.cap, cap_slot, EquipMouseIndex::cap);
  render_equip_texture(self.accessory, earacc_slot, EquipMouseIndex::earcc);
  render_equip_texture(self.coat, clothes_slot, EquipMouseIndex::clothes);
  render_equip_texture(self.pant, pants_slot, EquipMouseIndex::pants);
  render_equip_texture(self.shoes, shoes_slot, EquipMouseIndex::shoes);
  render_equip_texture(self.glove, gloves_slot, EquipMouseIndex::gloves);
  render_equip_texture(self.cape, cape_slot, EquipMouseIndex::cape);
  render_equip_texture(self.shield, shield_slot, EquipMouseIndex::shield);
  render_equip_texture(self.weapon, weapon_slot, EquipMouseIndex::weapon);
}

void EquipUiSystem::render_disable_texture(SDL_FPoint slot) {
  const SDL_FPoint lt{4, 45};
  static auto icon = WzResource::load_texture(
      WzResource::ui->find(u"Equipment.img/equip/canvas:disabled0"));
  auto x = (int)pos.x + slot.x + lt.x;
  auto y = (int)pos.y + slot.y + lt.y;
  SDL_FRect pos_rect{
      x,
      y,
      static_cast<float>(icon->w),
      static_cast<float>(icon->h),
  };
  SDL_RenderTexture(Window::renderer, icon, nullptr, &pos_rect);
}

void EquipUiSystem::render_deco_texture(std::optional<GameDecoItem> &deco,
                                          SDL_FPoint slot,
                                          EquipMouseIndex i) {
  const SDL_FPoint lt{4, 45};
  SDL_FRect pos_rect;
  pos_rect.x = (int)pos.x + slot.x + lt.x;
  pos_rect.y = (int)pos.y + slot.y + lt.y;
  pos_rect.w = 32;
  pos_rect.h = 32;
  const auto &mouse_pos = Window::mouse_pos;
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    mouse_index = i;
  }
  if (!deco.has_value()) {
    return;
  }
  auto info = EquipGameInstance::load_equip_info(deco->id);
  auto icon = WzResource::load_texture(info->get_child(u"icon"));
  auto x = (int)pos.x + slot.x + lt.x + (32 - icon->w) / 2;
  auto y = (int)pos.y + slot.y + lt.y + (32 - icon->h) / 2;
  pos_rect = {
      x,
      y,
      static_cast<float>(icon->w),
      static_cast<float>(icon->h),
  };
  SDL_RenderTexture(Window::renderer, icon, nullptr, &pos_rect);
  icon = WzResource::load_texture(
      WzResource::ms->get_root()->find(u"UI.img/CashItem"));
  pos_rect.x = (int)pos.x + slot.x + lt.x + 19;
  pos_rect.y = (int)pos.y + slot.y + lt.y + 19;
  pos_rect.w = icon->w;
  pos_rect.h = icon->h;
  SDL_RenderTexture(Window::renderer, icon, nullptr, &pos_rect);
}

void EquipUiSystem::render_deco() {
  mouse_index = std::nullopt;
  auto &self = CharacterGameInstance::self;
  render_deco_texture(self.cap_deco, cap_slot, EquipMouseIndex::cap);
  render_deco_texture(self.accessory_deco, earacc_slot,
                      EquipMouseIndex::earcc);
  render_deco_texture(self.coat_deco, clothes_slot, EquipMouseIndex::clothes);
  render_deco_texture(self.pant_deco, pants_slot, EquipMouseIndex::pants);
  render_deco_texture(self.shoes_deco, shoes_slot, EquipMouseIndex::shoes);
  render_deco_texture(self.glove_deco, gloves_slot, EquipMouseIndex::gloves);
  render_deco_texture(self.cape_deco, cape_slot, EquipMouseIndex::cape);
  render_deco_texture(self.shield_deco, shield_slot, EquipMouseIndex::shield);
  render_deco_texture(self.weapon_deco, weapon_slot, EquipMouseIndex::weapon);
  bool weapon_deco = false;
  if (self.weapon_deco.has_value()) {
    if (self.weapon_deco.has_value()) {
      if (self.weapon.has_value()) {
        std::u16string sub = self.weapon->id.substr(2, 2);
        std::u16string deco_val = self.weapon_deco->id + u"/" + sub;
        if (CharacterGameInstance::avatar_data.contains(deco_val)) {
          weapon_deco = true;
        }
      }
    }
    if (!weapon_deco) {
      render_disable_texture(weapon_slot);
    }
  }
}

bool EquipUiSystem::render_info() {
  auto index = mouse_index;
  if (index.has_value() && !CursorGameInstance::modal_overlay) {
    auto &mouse_pos = Window::mouse_pos;
    SDL_FPoint show_pos = {mouse_pos.x + 15, mouse_pos.y + 15};
    if (active_tab == 0) {
      auto equip = EquipGameInstance::load_equip((int)index.value());
      if (equip->has_value()) {
        TooltipUiSystem::render_equip(equip->value(), show_pos.x, show_pos.y);
      }
    } else {
      auto deco = EquipGameInstance::load_deco((int)index.value());
      if (deco->has_value()) {
        TooltipUiSystem::render_deco(deco->value(), show_pos.x, show_pos.y);
      }
    }
  }
  return true;
}

void EquipUiSystem::render_tab() {
  static auto node = WzResource::ui->find(u"Equipment.img/tab:mainTab");
  const SDL_FPoint lt{5, 26};
  for (auto i : {0, 1}) {
    SDL_Texture *texture;
    if (active_tab == i) {
      texture = WzResource::load_texture(
          node->get_child(u"selected")->get_child(std::to_string(i)));
    } else {
      texture = WzResource::load_texture(
          node->get_child(u"normal")->get_child(std::to_string(i)));
    }
    SDL_FRect pos_rect = {
        int(pos.x) + lt.x + texture->w * i,
        int(pos.y) + lt.y,
        static_cast<float>(texture->w),
        static_cast<float>(texture->h),
    };
    SDL_RenderTexture(Window::renderer, texture, nullptr, &pos_rect);
  }
}

void EquipUiSystem::render_backgrnd2() {
  const SDL_FPoint lt{4, 45};
  if (active_tab == 0) {
    static auto texture = WzResource::load_texture(
        WzResource::ui->find(u"Equipment.img/equip/backgrnd"));
    SDL_FRect pos_rect = {
        int(pos.x) + lt.x,
        int(pos.y) + lt.y,
        static_cast<float>(texture->w),
        static_cast<float>(texture->h),
    };
    SDL_RenderTexture(Window::renderer, texture, nullptr, &pos_rect);
  } else {
    static auto texture = WzResource::load_texture(
        WzResource::ui->find(u"Equipment.img/cash/backgrnd"));
    SDL_FRect pos_rect = {
        int(pos.x) + lt.x,
        int(pos.y) + lt.y,
        static_cast<float>(texture->w),
        static_cast<float>(texture->h),
    };
    SDL_RenderTexture(Window::renderer, texture, nullptr, &pos_rect);
  }
}

void EquipUiSystem::render_button() {
  const static std::array buttons_nodes = {
      WzResource::ui->find(u"Basic.img/BtClose"),
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
    auto &mouse_pos = Window::mouse_pos;
    // 判断按钮是否被遮挡
    auto cursor_in = CursorGameInstance::cursor_ui;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && cursor_in == render &&
        !CursorGameInstance::modal_overlay) {
      if (Window::mouse_state & SDL_BUTTON_LMASK) {
        auto pressed = WzResource::load_texture(k->find(u"pressed/0"));
        SDL_RenderTexture(Window::renderer, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = WzResource::load_texture(k->find(u"mouseOver/0"));
        SDL_RenderTexture(Window::renderer, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = WzResource::load_texture(k->find(u"normal/0"));
      SDL_RenderTexture(Window::renderer, normal, nullptr, &pos_rect);
    }
  }
}

bool EquipUiSystem::render() {
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

bool EquipUiSystem::event_click_equip(SDL_Event *event) {
  if (CursorGameInstance::cursor_hand_net.has_value()) {
    return false;
  }
  if (!mouse_index.has_value()) {
    return false;
  }
  auto index = (int)mouse_index.value();
  auto &self = CharacterGameInstance::self;
  auto &cursor_hand = CursorGameInstance::cursor_hand;
  if (cursor_hand.has_value()) {
    AudioGameInstance::load_audio(u"UI.img/DragEnd", 0);
    switch (cursor_hand->type) {
    case CursorGameInstance::equipment: {
      if (index == cursor_hand->sub_val && active_tab == 0) {
        EquipGameInstance::unuse_equip(index);
      }
      CursorGameInstance::cursor_hand = std::nullopt;
      break;
    }
    case CursorGameInstance::package: {
      switch ((ItemEnum)cursor_hand->val) {
      case ItemEnum::equip: {
        EquipGameInstance::use_equip(cursor_hand->sub_val);
        break;
      }
      case ItemEnum::deco: {
        EquipGameInstance::use_deco(cursor_hand->sub_val);
        CharacterLogicSystem::cct.map_id = SceneSystemInstance::map_id;
        break;
      }
      case ItemEnum::consume: {
        auto &itm = PackageGameInstance::data[(int)ItemEnum::consume]
                                               [cursor_hand->sub_val];
        if (itm->id.starts_with(u"0204")) {
          if (active_tab == 0) {
            auto eqp = EquipGameInstance::load_equip(index);
            if (eqp->has_value()) {
              auto &equip = eqp->value();
              auto &it = static_cast<GameConsumeItem &>(*itm);
              EquipGameInstance::use_equip_scroll(equip, it);
              CursorGameInstance::cursor_hand = std::nullopt;
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
    case CursorGameInstance::deco: {
      if (index == cursor_hand->sub_val && active_tab == 1) {
        EquipGameInstance::unuse_deco(index);
      }
      CursorGameInstance::cursor_hand = std::nullopt;

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
        click = EquipGameInstance::load_equip(index)->has_value();
      } else {
        click = EquipGameInstance::load_deco(index)->has_value();
      }
      if (click) {
        AudioGameInstance::load_audio(u"UI.img/DragStart", 0);
        if (active_tab == 0) {
          CursorGameInstance::cursor_hand = {
              .type = CursorGameInstance::equipment,
              .val = active_tab,
              .sub_val = static_cast<uint32_t>(index),
          };
        } else {
          CursorGameInstance::cursor_hand = {
              .type = CursorGameInstance::deco,
              .val = active_tab,
              .sub_val = static_cast<uint32_t>(index),
          };
        }
      }
    }
  }
  return false;
}

void EquipUiSystem::open() {
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::render);
  if (it != System::render_systems.end()) {
    auto wh = load_wh();
    auto &camera = CameraGameInstance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    System::render_systems.insert(it, render);
    System::event_systems.insert(System::event_systems.begin(), event);

    event_motion(nullptr);
  }
}

void EquipUiSystem::close() {
  std::erase(System::render_systems, render);
  std::erase(System::render_systems, render_info);
  std::erase(System::event_systems, event);

  event_drag_end();
}

void EquipUiSystem::toggle() {
  AudioGameInstance::load_audio(u"UI.img/BtMouseClick", 0);
  auto fn = &render;
  if (std::ranges::contains(System::render_systems, fn)) {
    close();
  } else {
    open();
  }
}

bool EquipUiSystem::cursor_in() {
  auto [w, h] = load_wh();
  auto &mouse = Window::mouse_pos;
  SDL_FRect pos_rect{pos.x, pos.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

void EquipUiSystem::event_top() {
  std::erase(System::render_systems, render);
  std::erase(System::event_systems, event);
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::render);
  if (it != System::render_systems.end()) {
    System::render_systems.insert(it, render);
    System::event_systems.insert(System::event_systems.begin(), event);

    event_motion(nullptr);
  }
}

void EquipUiSystem::event_drag_start(SDL_Event *event) {
  auto wh = load_wh();
  SDL_FRect pos_rect = {pos.x, pos.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag = {pos.x - event->button.x, pos.y - event->button.y};
  }
  return;
}

void EquipUiSystem::event_drag_end() {
  drag = std::nullopt;
  return;
}

void EquipUiSystem::event_drag_move(SDL_Event *event) {
  if (drag.has_value()) {
    pos = {event->motion.x + drag->x, event->motion.y + drag->y};
    auto &camera = CameraGameInstance::camera;
    auto [w, h] = load_wh();
    pos.x = std::clamp(pos.x, (float)0, camera.w - w);
    pos.y = std::clamp(pos.y, (float)0, camera.h - h);
  }
  return;
}

void EquipUiSystem::event_close() { close(); }

bool EquipUiSystem::event_button(SDL_Event *event) {
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
    if (SDL_PointInRectFloat(&Window::mouse_pos, &pos_rect)) {
      fns[i]();
      return true;
    }
  }

  return false;
}

void EquipUiSystem::event_tab(SDL_Event *event) {
  const static std::array tab_rect = {
      SDL_FRect{5, 26, 33, 19},  //
      SDL_FRect{38, 26, 33, 19}, //
  };
  for (uint8_t i = 0; i < tab_rect.size(); i++) {
    auto pos_rect = tab_rect[i];
    pos_rect.x += pos.x;
    pos_rect.y += pos.y;
    if (SDL_PointInRectFloat(&Window::mouse_pos, &pos_rect)) {
      active_tab = i;
    }
  }
}

void EquipUiSystem::event_motion(SDL_Event *event) {
  auto &sys = System::render_systems;
  std::erase(sys, render_info);
  auto it = std::ranges::find(sys, &CursorRenderSystem::render);
  if (it != sys.end()) {
    sys.insert(it, render_info);
  }
}

void EquipUiSystem::event_motion_bt_audio(SDL_Event *event) {
  if (CursorGameInstance::cursor_ui != render) {
    return;
  }
  std::vector<SDL_FRect> buttons_rect;
  auto wh = load_wh();
  buttons_rect = {
      SDL_FRect{wh.x - 20, 7, 12, 12}, //
  };
  static int8_t bt_audio = -1;
  auto &mouse_pos = Window::mouse_pos;
  int8_t bt_in = -1;
  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += (int)pos.x;
    pos_rect.y += (int)pos.y;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      bt_in = i;
      break;
    }
  }
  if (bt_in != -1 && bt_audio != bt_in) {
    AudioGameInstance::load_audio(u"UI.img/BtMouseOver", 0);
  }
  bt_audio = bt_in;
}

bool EquipUiSystem::event(SDL_Event *event) {
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
      if (CursorGameInstance::cursor_ui == render) {
        event_top();
        event_drag_start(event);
        r = false;
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == render) {
        event_tab(event);
        event_click_equip(event);
        r = !event_button(event);
      }
      event_drag_end();
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    event_motion_bt_audio(event);
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
