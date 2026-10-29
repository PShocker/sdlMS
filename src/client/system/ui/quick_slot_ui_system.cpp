#include "src/client/system/ui/quick_slot_ui_system.h"

#include "src/client/game/game_input.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/keyboard_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/package_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include <flat_map>
#include <format>
#include <optional>
#include <string>

namespace sdlms {

void QuickSlotUiSystem::RenderInputIcon(const GameInput& i, int x, int y,
                                        uint8_t alpha) {
  SDL_Texture* t;
  std::u16string id;
  if (i.type == "skill") {
    id = {i.val.begin(), i.val.end()};
    auto ski_info = SkillGameInstance::LoadSkiNode(id);
    t = WzResource::LoadTexture(ski_info->get_child(u"icon"));
  } else if (i.type == "item") {
    id = {i.val.begin(), i.val.end()};
    auto item_info = ItemGameInstance::LoadItemInfo(id, 1);
    t = WzResource::LoadTexture(item_info->get_child(u"icon"));
  } else if (i.type == "action") {
    const static std::flat_map<std::string, SDL_Texture*> actions = {
        {"pick", WzResource::LoadTexture(
                     WzResource::ui_->find(u"KeyBindings.img/icon/50"))},
        {"sit", WzResource::LoadTexture(
                    WzResource::ui_->find(u"KeyBindings.img/icon/51"))},
        {"attack", WzResource::LoadTexture(
                       WzResource::ui_->find(u"KeyBindings.img/icon/52"))},
        {"jump", WzResource::LoadTexture(
                     WzResource::ui_->find(u"KeyBindings.img/icon/53"))},
        {"up", WzResource::LoadTexture(
                   WzResource::ui_->find(u"KeyBindings.img/icon/302"))},
        {"down", WzResource::LoadTexture(
                     WzResource::ui_->find(u"KeyBindings.img/icon/303"))},
        {"left", WzResource::LoadTexture(
                     WzResource::ui_->find(u"KeyBindings.img/icon/300"))},
        {"right", WzResource::LoadTexture(
                      WzResource::ui_->find(u"KeyBindings.img/icon/301"))},
        {"harvest", WzResource::LoadTexture(
                        WzResource::ui_->find(u"KeyBindings.img/icon/54"))},
    };
    if (actions.contains(i.val)) {
      t = actions.at(i.val);
    }
  } else if (i.type == "ui") {
    const static std::flat_map<std::string, SDL_Texture*> uis = {
        {"equip", WzResource::LoadTexture(
                      WzResource::ui_->find(u"KeyBindings.img/icon/0"))},
        {"package", WzResource::LoadTexture(
                        WzResource::ui_->find(u"KeyBindings.img/icon/1"))},
        {"characterStat", WzResource::LoadTexture(WzResource::ui_->find(
                              u"KeyBindings.img/icon/2"))},
        {"skill", WzResource::LoadTexture(
                      WzResource::ui_->find(u"KeyBindings.img/icon/3"))},
        {"buddy", WzResource::LoadTexture(
                      WzResource::ui_->find(u"KeyBindings.img/icon/4"))},
        {"worldMap", WzResource::LoadTexture(
                         WzResource::ui_->find(u"KeyBindings.img/icon/5"))},
        {"mapleChat", WzResource::LoadTexture(
                          WzResource::ui_->find(u"KeyBindings.img/icon/6"))},
        {"miniMap", WzResource::LoadTexture(
                        WzResource::ui_->find(u"KeyBindings.img/icon/7"))},
        {"quest", WzResource::LoadTexture(
                      WzResource::ui_->find(u"KeyBindings.img/icon/8"))},
        {"keyBind", WzResource::LoadTexture(
                        WzResource::ui_->find(u"KeyBindings.img/icon/9"))},
        {"toAll", WzResource::LoadTexture(
                      WzResource::ui_->find(u"KeyBindings.img/icon/10"))},
        {"whisper", WzResource::LoadTexture(
                        WzResource::ui_->find(u"KeyBindings.img/icon/11"))},
        {"toParty", WzResource::LoadTexture(
                        WzResource::ui_->find(u"KeyBindings.img/icon/12"))},
        {"toBuddy", WzResource::LoadTexture(
                        WzResource::ui_->find(u"KeyBindings.img/icon/13"))},
        {"shortCut", WzResource::LoadTexture(
                         WzResource::ui_->find(u"KeyBindings.img/icon/14"))},
        {"quickSlot", WzResource::LoadTexture(
                          WzResource::ui_->find(u"KeyBindings.img/icon/15"))},
        {"chat+", WzResource::LoadTexture(
                      WzResource::ui_->find(u"KeyBindings.img/icon/16"))},
        {"party", WzResource::LoadTexture(
                      WzResource::ui_->find(u"KeyBindings.img/icon/19"))},
        {"cashShop", WzResource::LoadTexture(
                         WzResource::ui_->find(u"KeyBindings.img/icon/22"))},
        {"changeChannel", WzResource::LoadTexture(WzResource::ui_->find(
                              u"KeyBindings.img/icon/45"))},
        {"menu", WzResource::LoadTexture(
                     WzResource::ui_->find(u"KeyBindings.img/icon/46"))},
        {"screenShot", WzResource::LoadTexture(
                           WzResource::ui_->find(u"KeyBindings.img/icon/47"))},
        {"picMode", WzResource::LoadTexture(
                        WzResource::ui_->find(u"KeyBindings.img/icon/48"))},
        {"craft", WzResource::LoadTexture(
                      WzResource::ui_->find(u"KeyBindings.img/icon/55"))},
        {"mailBox", WzResource::LoadTexture(
                        WzResource::ui_->find(u"KeyBindings.img/icon/58"))},
        {"ok", WzResource::LoadTexture(
                   WzResource::ui_->find(u"KeyBindings.img/icon/400"))},
    };
    if (uis.contains(i.val)) {
      t = uis.at(i.val);
    }
  } else if (i.type == "face") {
    const static std::flat_map<std::string, SDL_Texture*> faces = {
        {"hit", WzResource::LoadTexture(
                    WzResource::ui_->find(u"KeyBindings.img/icon/100"))},
        {"smile", WzResource::LoadTexture(
                      WzResource::ui_->find(u"KeyBindings.img/icon/101"))},
        {"troubled", WzResource::LoadTexture(
                         WzResource::ui_->find(u"KeyBindings.img/icon/102"))},
        {"cry", WzResource::LoadTexture(
                    WzResource::ui_->find(u"KeyBindings.img/icon/103"))},
        {"angry", WzResource::LoadTexture(
                      WzResource::ui_->find(u"KeyBindings.img/icon/104"))},
        {"bewildered", WzResource::LoadTexture(
                           WzResource::ui_->find(u"KeyBindings.img/icon/105"))},
        {"stunned", WzResource::LoadTexture(
                        WzResource::ui_->find(u"KeyBindings.img/icon/106"))},
    };
    if (faces.contains(i.val)) {
      t = faces.at(i.val);
    }
  }
  if (t == nullptr) {
    return;
  }
  SDL_FRect pos_rect{
      static_cast<float>(x),
      static_cast<float>(y),
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  if (i.type == "item") {
    pos_rect.x += (32 - pos_rect.w) / 2;
    pos_rect.y += (32 - pos_rect.h) / 2;
  }
  SDL_SetTextureAlphaMod(t, alpha);
  SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
  SDL_SetTextureAlphaMod(t, 255);
  if (i.type == "item") {
    auto num = PackageGameInstance::LoadItemNum(id);
    PackageUiSystem::RenderNumber(num, x, y + 16);
  }
}

void QuickSlotUiSystem::RenderQuickSlot() {
  const static std::flat_map<SDL_Scancode, SDL_Texture*> textures = {
      {SDL_SCANCODE_PAGEDOWN,
       WzResource::LoadTexture(WzResource::ui_->find(u"QuickSlot.img/key/81"))},
      {SDL_SCANCODE_END,
       WzResource::LoadTexture(WzResource::ui_->find(u"QuickSlot.img/key/79"))},
      {SDL_SCANCODE_PAGEUP,
       WzResource::LoadTexture(WzResource::ui_->find(u"QuickSlot.img/key/73"))},
      {SDL_SCANCODE_HOME,
       WzResource::LoadTexture(WzResource::ui_->find(u"QuickSlot.img/key/71"))},
      {SDL_SCANCODE_INSERT,
       WzResource::LoadTexture(WzResource::ui_->find(u"QuickSlot.img/key/82"))},
      {SDL_SCANCODE_DELETE,
       WzResource::LoadTexture(WzResource::ui_->find(u"QuickSlot.img/key/83"))},
      {SDL_SCANCODE_LCTRL,
       WzResource::LoadTexture(WzResource::ui_->find(u"QuickSlot.img/key/29"))},
      {SDL_SCANCODE_LSHIFT,
       WzResource::LoadTexture(WzResource::ui_->find(u"QuickSlot.img/key/42"))},
  };
  switch (quickSlot_) {
    case QuickSlot::kHide: {
      break;
    }
    case QuickSlot::kTwo: {
      static auto q = WzResource::LoadTexture(
          WzResource::ui_->find(u"QuickSlot.img/layer:backgrnd"));
      auto screen_w = CameraGameInstance::camera_.w;
      auto screen_h = CameraGameInstance::camera_.h;
      auto base_x = (screen_w - 808) / 2;
      auto base_y = (screen_h - 73);
      SDL_FRect p{
          base_x + 654,
          base_y - 107,
          static_cast<float>(q->w),
          static_cast<float>(q->h),
      };
      SDL_RenderTexture(Window::renderer_, q, nullptr, &p);
      std::vector<SDL_Scancode> scode = {
          (SDL_SCANCODE_LSHIFT), (SDL_SCANCODE_INSERT),   (SDL_SCANCODE_HOME),
          (SDL_SCANCODE_PAGEUP), (SDL_SCANCODE_LCTRL),    (SDL_SCANCODE_DELETE),
          (SDL_SCANCODE_END),    (SDL_SCANCODE_PAGEDOWN),
      };
      std::vector<SDL_FPoint> r = {
          SDL_FPoint{p.x + 9, p.y + 10},
          SDL_FPoint{p.x + 44, p.y + 10},
          SDL_FPoint{p.x + 79, p.y + 10},
          SDL_FPoint{p.x + 114, p.y + 10},
          // 2row
          SDL_FPoint{p.x + 9, p.y + 44},
          SDL_FPoint{p.x + 44, p.y + 44},
          SDL_FPoint{p.x + 79, p.y + 44},
          SDL_FPoint{p.x + 114, p.y + 44},
      };
      for (int i = 0; i < r.size(); i++) {
        SDL_FRect pos_rect;
        if (KeyboardGameInstance::data_.contains(scode[i])) {
          const auto& input = KeyboardGameInstance::data_.at(scode[i]);
          RenderInputIcon(input, r[i].x - 2, r[i].y + 32);
        }
        auto texture = textures.at(scode[i]);
        pos_rect = {
            r[i].x,
            r[i].y + 34,
            static_cast<float>(texture->w),
            static_cast<float>(texture->h),
        };
        SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
      }

      break;
    }
    case QuickSlot::kThree: {
      break;
    }
  }
}

bool QuickSlotUiSystem::Render() {
  RenderQuickSlot();
  return true;
}

SDL_FPoint QuickSlotUiSystem::LoadWh() {
  switch (quickSlot_) {
    case QuickSlot::kHide:
      return {0, 0};
    case QuickSlot::kTwo:
      return {151, 78};
    case QuickSlot::kThree:
      return {151, 114};
  }
  return {0, 0};
}

void QuickSlotUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

void QuickSlotUiSystem::Close() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
}

void QuickSlotUiSystem::Toggle() {
  auto fn = &Render;
  if (std::ranges::contains(System::render_systems_, fn)) {
    Close();
  } else {
    Open();
  }
}

bool QuickSlotUiSystem::CursorIn() {
  auto [w, h] = LoadWh();
  auto& mouse = Window::mouse_pos_;
  auto screen_w = CameraGameInstance::camera_.w;
  auto screen_h = CameraGameInstance::camera_.h;
  auto base_x = (screen_w - 808) / 2;
  auto base_y = (screen_h - 73);
  base_x = base_x + 654;
  base_y = base_y - 107;
  SDL_FRect pos_rect{
      base_x,
      base_y,
      w,
      h,
  };
  if (quickSlot_ == QuickSlot::kTwo) {
    pos_rect.y += 36;
  }
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

bool QuickSlotUiSystem::EventClickQuickslot(SDL_Event* event) {
  if (CursorGameInstance::cursor_hand_net_.has_value()) {
    return false;
  }
  auto screen_w = CameraGameInstance::camera_.w;
  auto screen_h = CameraGameInstance::camera_.h;
  auto base_x = (screen_w - 808) / 2;
  auto base_y = (screen_h - 73);
  switch (quickSlot_) {
    case QuickSlot::kHide: {
      break;
    }
    case QuickSlot::kTwo: {
      SDL_FPoint p{
          base_x + 654,
          base_y - 107,
      };
      std::vector<SDL_Scancode> t = {
          SDL_SCANCODE_LSHIFT, SDL_SCANCODE_INSERT,   SDL_SCANCODE_HOME,
          SDL_SCANCODE_PAGEUP, SDL_SCANCODE_LCTRL,    SDL_SCANCODE_DELETE,
          SDL_SCANCODE_END,    SDL_SCANCODE_PAGEDOWN,
      };
      std::vector<SDL_FPoint> r = {
          SDL_FPoint{p.x + 9, p.y + 10},
          SDL_FPoint{p.x + 44, p.y + 10},
          SDL_FPoint{p.x + 79, p.y + 10},
          SDL_FPoint{p.x + 114, p.y + 10},
          // 2row
          SDL_FPoint{p.x + 9, p.y + 44},
          SDL_FPoint{p.x + 44, p.y + 44},
          SDL_FPoint{p.x + 79, p.y + 44},
          SDL_FPoint{p.x + 114, p.y + 44},
      };
      for (int i = 0; i < r.size(); i++) {
        SDL_FRect pos_rect{
            r[i].x,
            r[i].y + 32,
            static_cast<float>(32),
            static_cast<float>(32),
        };
        if (SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect)) {
          if (CursorGameInstance::cursor_hand_.has_value()) {
            auto input = KeyboardGameInstance::LoadCursorInput();
            KeyboardGameInstance::InsertInput(t[i], input.value());
            CursorGameInstance::cursor_hand_ = std::nullopt;
            AudioGameInstance::LoadAudio(u"UI.img/DragEnd", 0);
          } else {
            if (KeyboardGameInstance::data_.contains(t[i])) {
              CursorGameInstance::cursor_hand_ = {
                  .type = CursorGameInstance::CursorHandType::kKeybind,
                  .val = t[i],
              };
              AudioGameInstance::LoadAudio(u"UI.img/DragStart", 0);
            }
          }
          return true;
        }
      }

      break;
    }
    case QuickSlot::kThree: {
      break;
    }
  }
  return false;
}

bool QuickSlotUiSystem::Event(SDL_Event* event) {
  bool r = true;
  switch (event->type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui_ == Render) {
          r = false;
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui_ == Render) {
          r = !EventClickQuickslot(event);
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      break;
    }
    default: {
      break;
    }
  }

  return r;
}

}  // namespace sdlms
