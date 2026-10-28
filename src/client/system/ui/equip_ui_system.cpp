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
namespace sdlms {


const static SDL_FPoint cap_slot{68, 23};
const static SDL_FPoint earacc_slot{101, 56};
const static SDL_FPoint clothes_slot{35, 89};
const static SDL_FPoint pants_slot{35, 122};
const static SDL_FPoint shoes_slot{35, 155};
const static SDL_FPoint gloves_slot{2, 122};
const static SDL_FPoint cape_slot{2, 89};
const static SDL_FPoint shield_slot{134, 89};
const static SDL_FPoint weapon_slot{101, 89};

SDL_FPoint EquipUiSystem::LoadWh() { return {175, 289}; }

void EquipUiSystem::RenderBackgrnd() {
  static auto backgrnd = WzResource::LoadTexture(
      WzResource::ui_->find(u"Equipment.img/backgrnd"));

  SDL_FRect pos_rect{pos_.x, pos_.y, static_cast<float>(backgrnd->w),
                     static_cast<float>(backgrnd->h)};
  SDL_RenderTexture(Window::renderer_, backgrnd, nullptr, &pos_rect);
}

void EquipUiSystem::RenderEquipTexture(
    std::optional<GameEquipItem> &equip, SDL_FPoint slot,
    EquipMouseIndex i) {
  const SDL_FPoint lt{4, 45};
  SDL_FRect pos_rect;
  pos_rect.x = (int)pos_.x + slot.x + lt.x;
  pos_rect.y = (int)pos_.y + slot.y + lt.y;
  pos_rect.w = 32;
  pos_rect.h = 32;
  const auto &mouse_pos = Window::mouse_pos_;
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    mouse_index_ = i;
  }
  if (!equip.has_value()) {
    return;
  }
  auto info = EquipGameInstance::LoadEquipInfo(equip->id);
  auto icon = WzResource::LoadTexture(info->get_child(u"icon"));
  auto x = (int)pos_.x + slot.x + lt.x + (32 - icon->w) / 2;
  auto y = (int)pos_.y + slot.y + lt.y + (32 - icon->h) / 2;
  pos_rect = {
      x,
      y,
      static_cast<float>(icon->w),
      static_cast<float>(icon->h),
  };
  SDL_RenderTexture(Window::renderer_, icon, nullptr, &pos_rect);
}

void EquipUiSystem::RenderEquip() {
  mouse_index_ = std::nullopt;
  auto &self = CharacterGameInstance::self_;
  RenderEquipTexture(self.cap, cap_slot, EquipMouseIndex::kCap);
  RenderEquipTexture(self.accessory, earacc_slot, EquipMouseIndex::kEarcc);
  RenderEquipTexture(self.coat, clothes_slot, EquipMouseIndex::kClothes);
  RenderEquipTexture(self.pant, pants_slot, EquipMouseIndex::kPants);
  RenderEquipTexture(self.shoes, shoes_slot, EquipMouseIndex::kShoes);
  RenderEquipTexture(self.glove, gloves_slot, EquipMouseIndex::kGloves);
  RenderEquipTexture(self.cape, cape_slot, EquipMouseIndex::kCape);
  RenderEquipTexture(self.shield, shield_slot, EquipMouseIndex::kShield);
  RenderEquipTexture(self.weapon, weapon_slot, EquipMouseIndex::kWeapon);
}

void EquipUiSystem::RenderDisableTexture(SDL_FPoint slot) {
  const SDL_FPoint lt{4, 45};
  static auto icon = WzResource::LoadTexture(
      WzResource::ui_->find(u"Equipment.img/equip/canvas:disabled0"));
  auto x = (int)pos_.x + slot.x + lt.x;
  auto y = (int)pos_.y + slot.y + lt.y;
  SDL_FRect pos_rect{
      x,
      y,
      static_cast<float>(icon->w),
      static_cast<float>(icon->h),
  };
  SDL_RenderTexture(Window::renderer_, icon, nullptr, &pos_rect);
}

void EquipUiSystem::RenderDecoTexture(std::optional<GameDecoItem> &deco,
                                          SDL_FPoint slot,
                                          EquipMouseIndex i) {
  const SDL_FPoint lt{4, 45};
  SDL_FRect pos_rect;
  pos_rect.x = (int)pos_.x + slot.x + lt.x;
  pos_rect.y = (int)pos_.y + slot.y + lt.y;
  pos_rect.w = 32;
  pos_rect.h = 32;
  const auto &mouse_pos = Window::mouse_pos_;
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    mouse_index_ = i;
  }
  if (!deco.has_value()) {
    return;
  }
  auto info = EquipGameInstance::LoadEquipInfo(deco->id);
  auto icon = WzResource::LoadTexture(info->get_child(u"icon"));
  auto x = (int)pos_.x + slot.x + lt.x + (32 - icon->w) / 2;
  auto y = (int)pos_.y + slot.y + lt.y + (32 - icon->h) / 2;
  pos_rect = {
      x,
      y,
      static_cast<float>(icon->w),
      static_cast<float>(icon->h),
  };
  SDL_RenderTexture(Window::renderer_, icon, nullptr, &pos_rect);
  icon = WzResource::LoadTexture(
      WzResource::ms_->get_root()->find(u"UI.img/CashItem"));
  pos_rect.x = (int)pos_.x + slot.x + lt.x + 19;
  pos_rect.y = (int)pos_.y + slot.y + lt.y + 19;
  pos_rect.w = icon->w;
  pos_rect.h = icon->h;
  SDL_RenderTexture(Window::renderer_, icon, nullptr, &pos_rect);
}

void EquipUiSystem::RenderDeco() {
  mouse_index_ = std::nullopt;
  auto &self = CharacterGameInstance::self_;
  RenderDecoTexture(self.cap_deco, cap_slot, EquipMouseIndex::kCap);
  RenderDecoTexture(self.accessory_deco, earacc_slot,
                      EquipMouseIndex::kEarcc);
  RenderDecoTexture(self.coat_deco, clothes_slot, EquipMouseIndex::kClothes);
  RenderDecoTexture(self.pant_deco, pants_slot, EquipMouseIndex::kPants);
  RenderDecoTexture(self.shoes_deco, shoes_slot, EquipMouseIndex::kShoes);
  RenderDecoTexture(self.glove_deco, gloves_slot, EquipMouseIndex::kGloves);
  RenderDecoTexture(self.cape_deco, cape_slot, EquipMouseIndex::kCape);
  RenderDecoTexture(self.shield_deco, shield_slot, EquipMouseIndex::kShield);
  RenderDecoTexture(self.weapon_deco, weapon_slot, EquipMouseIndex::kWeapon);
  bool weapon_deco = false;
  if (self.weapon_deco.has_value()) {
    if (self.weapon_deco.has_value()) {
      if (self.weapon.has_value()) {
        std::u16string sub = self.weapon->id.substr(2, 2);
        std::u16string deco_val = self.weapon_deco->id + u"/" + sub;
        if (CharacterGameInstance::avatar_data_.contains(deco_val)) {
          weapon_deco = true;
        }
      }
    }
    if (!weapon_deco) {
      RenderDisableTexture(weapon_slot);
    }
  }
}

bool EquipUiSystem::RenderInfo() {
  auto index = mouse_index_;
  if (index.has_value() && !CursorGameInstance::modal_overlay) {
    auto &mouse_pos = Window::mouse_pos_;
    SDL_FPoint show_pos = {mouse_pos.x + 15, mouse_pos.y + 15};
    if (active_tab_ == 0) {
      auto equip = EquipGameInstance::LoadEquip((int)index.value());
      if (equip->has_value()) {
        TooltipUiSystem::RenderEquip(equip->value(), show_pos.x, show_pos.y);
      }
    } else {
      auto deco = EquipGameInstance::LoadDeco((int)index.value());
      if (deco->has_value()) {
        TooltipUiSystem::RenderDeco(deco->value(), show_pos.x, show_pos.y);
      }
    }
  }
  return true;
}

void EquipUiSystem::RenderTab() {
  static auto node = WzResource::ui_->find(u"Equipment.img/tab:mainTab");
  const SDL_FPoint lt{5, 26};
  for (auto i : {0, 1}) {
    SDL_Texture *texture;
    if (active_tab_ == i) {
      texture = WzResource::LoadTexture(
          node->get_child(u"selected")->get_child(std::to_string(i)));
    } else {
      texture = WzResource::LoadTexture(
          node->get_child(u"normal")->get_child(std::to_string(i)));
    }
    SDL_FRect pos_rect = {
        int(pos_.x) + lt.x + texture->w * i,
        int(pos_.y) + lt.y,
        static_cast<float>(texture->w),
        static_cast<float>(texture->h),
    };
    SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
  }
}

void EquipUiSystem::RenderBackgrnd2() {
  const SDL_FPoint lt{4, 45};
  if (active_tab_ == 0) {
    static auto texture = WzResource::LoadTexture(
        WzResource::ui_->find(u"Equipment.img/equip/backgrnd"));
    SDL_FRect pos_rect = {
        int(pos_.x) + lt.x,
        int(pos_.y) + lt.y,
        static_cast<float>(texture->w),
        static_cast<float>(texture->h),
    };
    SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
  } else {
    static auto texture = WzResource::LoadTexture(
        WzResource::ui_->find(u"Equipment.img/cash/backgrnd"));
    SDL_FRect pos_rect = {
        int(pos_.x) + lt.x,
        int(pos_.y) + lt.y,
        static_cast<float>(texture->w),
        static_cast<float>(texture->h),
    };
    SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
  }
}

void EquipUiSystem::RenderButton() {
  const static std::array buttons_nodes = {
      WzResource::ui_->find(u"Basic.img/BtClose"),
  };
  auto wh = LoadWh();
  std::array buttons_rect = {
      SDL_FRect{wh.x - 20, 7, 12, 12}, //
  };

  for (size_t i = 0; i < buttons_nodes.size(); ++i) {
    auto k = buttons_nodes[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x += pos_.x;
    pos_rect.y += pos_.y;
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    auto &mouse_pos = Window::mouse_pos_;
    // 判断按钮是否被遮挡
    auto CursorIn = CursorGameInstance::cursor_ui;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && CursorIn == Render &&
        !CursorGameInstance::modal_overlay) {
      if (Window::mouse_state_ & SDL_BUTTON_LMASK) {
        auto pressed = WzResource::LoadTexture(k->find(u"pressed/0"));
        SDL_RenderTexture(Window::renderer_, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = WzResource::LoadTexture(k->find(u"mouseOver/0"));
        SDL_RenderTexture(Window::renderer_, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = WzResource::LoadTexture(k->find(u"normal/0"));
      SDL_RenderTexture(Window::renderer_, normal, nullptr, &pos_rect);
    }
  }
}

bool EquipUiSystem::Render() {
  RenderBackgrnd();
  RenderBackgrnd2();
  RenderTab();
  RenderButton();

  if (active_tab_ == 0) {
    RenderEquip();
  } else {
    RenderDeco();
  }

  return true;
}

bool EquipUiSystem::EventClickEquip(SDL_Event *Event) {
  if (CursorGameInstance::cursor_hand_net_.has_value()) {
    return false;
  }
  if (!mouse_index_.has_value()) {
    return false;
  }
  auto index = (int)mouse_index_.value();
  auto &self = CharacterGameInstance::self_;
  auto &cursor_hand = CursorGameInstance::cursor_hand_;
  if (cursor_hand.has_value()) {
    AudioGameInstance::LoadAudio(u"UI.img/DragEnd", 0);
    switch (cursor_hand->type) {
    case CursorGameInstance::CursorHandType::kEquipment: {
      if (index == cursor_hand->sub_val && active_tab_ == 0) {
        EquipGameInstance::UnuseEquip(index);
      }
      CursorGameInstance::cursor_hand_ = std::nullopt;
      break;
    }
    case CursorGameInstance::CursorHandType::kPackage: {
      switch ((ItemEnum)cursor_hand->val) {
      case ItemEnum::kEquip: {
        EquipGameInstance::UseEquip(cursor_hand->sub_val);
        break;
      }
      case ItemEnum::kDeco: {
        EquipGameInstance::UseDeco(cursor_hand->sub_val);
        CharacterLogicSystem::cct_.map_id = SceneSystemInstance::map_id_;
        break;
      }
      case ItemEnum::kConsume: {
        auto &itm = PackageGameInstance::data_[(int)ItemEnum::kConsume]
                                               [cursor_hand->sub_val];
        if (itm->id.starts_with(u"0204")) {
          if (active_tab_ == 0) {
            auto eqp = EquipGameInstance::LoadEquip(index);
            if (eqp->has_value()) {
              auto &equip = eqp->value();
              auto &it = static_cast<GameConsumeItem &>(*itm);
              EquipGameInstance::UseEquipScroll(equip, it);
              CursorGameInstance::cursor_hand_ = std::nullopt;
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
    case CursorGameInstance::CursorHandType::kDeco: {
      if (index == cursor_hand->sub_val && active_tab_ == 1) {
        EquipGameInstance::UnuseDeco(index);
      }
      CursorGameInstance::cursor_hand_ = std::nullopt;

      break;
    }
    default: {
      break;
    }
    }
  } else {
    if (Event->button.button == SDL_BUTTON_LEFT) {
      bool Click = false;
      if (active_tab_ == 0) {
        Click = EquipGameInstance::LoadEquip(index)->has_value();
      } else {
        Click = EquipGameInstance::LoadDeco(index)->has_value();
      }
      if (Click) {
        AudioGameInstance::LoadAudio(u"UI.img/DragStart", 0);
        if (active_tab_ == 0) {
          CursorGameInstance::cursor_hand_ = {
              .type = CursorGameInstance::CursorHandType::kEquipment,
              .val = active_tab_,
              .sub_val = static_cast<uint32_t>(index),
          };
        } else {
          CursorGameInstance::cursor_hand_ = {
              .type = CursorGameInstance::CursorHandType::kDeco,
              .val = active_tab_,
              .sub_val = static_cast<uint32_t>(index),
          };
        }
      }
    }
  }
  return false;
}

void EquipUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    auto wh = LoadWh();
    auto &camera = CameraGameInstance::camera_;
    pos_.x = (camera.w - wh.x) / 2;
    pos_.y = (camera.h - wh.y) / 2;

    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);

    EventMotion(nullptr);
  }
}

void EquipUiSystem::Close() {
  std::erase(System::render_systems_, Render);
  std::erase(System::render_systems_, RenderInfo);
  std::erase(System::event_systems_, Event);

  EventDragEnd();
}

void EquipUiSystem::Toggle() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
  auto fn = &Render;
  if (std::ranges::contains(System::render_systems_, fn)) {
    Close();
  } else {
    Open();
  }
}

bool EquipUiSystem::CursorIn() {
  auto [w, h] = LoadWh();
  auto &mouse = Window::mouse_pos_;
  SDL_FRect pos_rect{pos_.x, pos_.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

void EquipUiSystem::EventTop() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);

    EventMotion(nullptr);
  }
}

void EquipUiSystem::EventDragStart(SDL_Event *Event) {
  auto wh = LoadWh();
  SDL_FRect pos_rect = {pos_.x, pos_.y, wh.x, 20};
  SDL_FPoint mouse_pos = {Event->button.x, Event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag_ = {pos_.x - Event->button.x, pos_.y - Event->button.y};
  }
  return;
}

void EquipUiSystem::EventDragEnd() {
  drag_ = std::nullopt;
  return;
}

void EquipUiSystem::EventDragMove(SDL_Event *Event) {
  if (drag_.has_value()) {
    pos_ = {Event->motion.x + drag_->x, Event->motion.y + drag_->y};
    auto &camera = CameraGameInstance::camera_;
    auto [w, h] = LoadWh();
    pos_.x = std::clamp(pos_.x, (float)0, camera.w - w);
    pos_.y = std::clamp(pos_.y, (float)0, camera.h - h);
  }
  return;
}

void EquipUiSystem::EventClose() { Close(); }

bool EquipUiSystem::EventButton(SDL_Event *Event) {
  std::vector<SDL_FRect> r;
  std::vector<void (*)()> fns;
  auto wh = LoadWh();
  r = {
      SDL_FRect{wh.x - 20, 7, 12, 12}, //
  };
  fns = {
      EventClose,
  };

  for (size_t i = 0; i < r.size(); ++i) {
    auto pos_rect = r[i];
    pos_rect.x += pos_.x;
    pos_rect.y += pos_.y;
    if (SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect)) {
      fns[i]();
      return true;
    }
  }

  return false;
}

void EquipUiSystem::EventTab(SDL_Event *Event) {
  const static std::array tab_rect = {
      SDL_FRect{5, 26, 33, 19},  //
      SDL_FRect{38, 26, 33, 19}, //
  };
  for (uint8_t i = 0; i < tab_rect.size(); i++) {
    auto pos_rect = tab_rect[i];
    pos_rect.x += pos_.x;
    pos_rect.y += pos_.y;
    if (SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect)) {
      active_tab_ = i;
    }
  }
}

void EquipUiSystem::EventMotion(SDL_Event *Event) {
  auto &sys = System::render_systems_;
  std::erase(sys, RenderInfo);
  auto it = std::ranges::find(sys, &CursorRenderSystem::Render);
  if (it != sys.end()) {
    sys.insert(it, RenderInfo);
  }
}

void EquipUiSystem::EventMotionBtAudio(SDL_Event *Event) {
  if (CursorGameInstance::cursor_ui != Render) {
    return;
  }
  std::vector<SDL_FRect> buttons_rect;
  auto wh = LoadWh();
  buttons_rect = {
      SDL_FRect{wh.x - 20, 7, 12, 12}, //
  };
  static int8_t bt_audio = -1;
  auto &mouse_pos = Window::mouse_pos_;
  int8_t bt_in = -1;
  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += (int)pos_.x;
    pos_rect.y += (int)pos_.y;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      bt_in = i;
      break;
    }
  }
  if (bt_in != -1 && bt_audio != bt_in) {
    AudioGameInstance::LoadAudio(u"UI.img/BtMouseOver", 0);
  }
  bt_audio = bt_in;
}

bool EquipUiSystem::Event(SDL_Event *Event) {
  bool r = true;
  switch (Event->type) {
  case SDL_EVENT_KEY_DOWN: {
    auto scan_code = Event->key.scancode;
    switch (scan_code) {
    case SDL_SCANCODE_ESCAPE: {
      EventClose();
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
    if (Event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == Render) {
        EventTop();
        EventDragStart(Event);
        r = false;
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (Event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == Render) {
        EventTab(Event);
        EventClickEquip(Event);
        r = !EventButton(Event);
      }
      EventDragEnd();
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    EventMotionBtAudio(Event);
    EventMotion(Event);
    EventDragMove(Event);
    break;
  }
  default: {
    break;
  }
  }

  return r;
}

}  // namespace sdlms
