#include "src/client/system/ui/craft_ui_system.h"

#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/component/tab.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <string>

namespace sdlms {

void CraftUiSystem::RenderBackgrnd() {
  static auto texture =
      WzResource::LoadTexture(WzResource::ui_->find(u"Crafting.img/backgrnd"));
  SDL_FRect pos_rect{pos_.x, pos_.y, static_cast<float>(texture->w),
                     static_cast<float>(texture->h)};
  SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
}

UIComponents& CraftUiSystem::GetComponent() {
  components_.clear();
  const static auto tab_node =
      WzResource::ui_->find(u"Crafting.img/tab:category");
  std::vector<SDL_FRect> rects;
  for (uint8_t i = 0; i <= 5; i++) {
    rects.push_back(
        SDL_FRect{static_cast<float>(int(pos_.x + 9 + i * 88)),
                  static_cast<float>(int(pos_.y + 23)), 88, 21});
  }
  components_.push_back(
      Tab{tab_node, u"selected", u"normal", rects, selected_tab_});
  return components_;
}

bool CraftUiSystem::EventUI(SDL_Event* event) {
  for (auto& component : GetComponent()) {
    if (auto* tab = std::get_if<Tab>(&component)) {
      if (auto i = tab->Click()) {
        selected_tab_ = *i;
        return true;
      }
    }
  }
  return false;
}

bool CraftUiSystem::Render() {
  RenderBackgrnd();
  RenderUI();
  return true;
}

SDL_FPoint CraftUiSystem::LoadWh() {
  return {545, 409};
}

void CraftUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    auto wh = LoadWh();
    auto& camera = CameraGameInstance::camera_;
    pos_.x = (camera.w - wh.x) / 2;
    pos_.y = (camera.h - wh.y) / 2;

    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

void CraftUiSystem::Close() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
}

void CraftUiSystem::EventTop() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

void CraftUiSystem::EventDragStart(SDL_Event* event) {
  auto wh = LoadWh();
  SDL_FRect pos_rect = {pos_.x, pos_.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag_ = {pos_.x - event->button.x, pos_.y - event->button.y};
  }
  return;
}

void CraftUiSystem::EventDragEnd() {
  drag_ = std::nullopt;
  return;
}

void CraftUiSystem::EventDragMove(SDL_Event* event) {
  if (drag_.has_value()) {
    pos_ = {event->motion.x + drag_->x, event->motion.y + drag_->y};
    auto& camera = CameraGameInstance::camera_;
    auto [w, h] = LoadWh();
    pos_.x = std::clamp(pos_.x, (float)0, camera.w - w);
    pos_.y = std::clamp(pos_.y, (float)0, camera.h - h);
  }
  return;
}

void CraftUiSystem::Toggle() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
  auto fn = &Render;
  if (std::ranges::contains(System::render_systems_, fn)) {
    Close();
  } else {
    Open();
  }
}

bool CraftUiSystem::CursorIn() {
  auto [w, h] = LoadWh();
  auto& mouse = Window::mouse_pos_;
  SDL_FRect pos_rect{pos_.x, pos_.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

bool CraftUiSystem::Event(SDL_Event* event) {
  bool r = true;
  switch (event->type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui_ == Render) {
          EventTop();
          EventDragStart(event);
          r = false;
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui_ == Render) {
          r = !EventUI(event);
        }
        EventDragEnd();
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      EventDragMove(event);
      break;
    }
    default: {
      break;
    }
  }

  return r;
}

bool CraftUiSystem::Run() {
  return true;
}

}  // namespace sdlms
