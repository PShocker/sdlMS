#include "src/client/system/ui/craft_ui_system.h"

#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
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

void CraftUiSystem::RenderTab() {
  const static SDL_FPoint lt = {9, 23};
  const static SDL_FPoint rb = {537, 44};
  const static auto tab_node =
      WzResource::ui_->find(u"Crafting.img/tab:category");
  const static std::array selected_texture = {
      WzResource::LoadTexture(tab_node->find(u"selected/0")),
      WzResource::LoadTexture(tab_node->find(u"selected/1")),
      WzResource::LoadTexture(tab_node->find(u"selected/2")),
      WzResource::LoadTexture(tab_node->find(u"selected/3")),
      WzResource::LoadTexture(tab_node->find(u"selected/4")),
      WzResource::LoadTexture(tab_node->find(u"selected/5")),
  };
  const static std::array normal_texture = {
      WzResource::LoadTexture(tab_node->find(u"normal/0")),
      WzResource::LoadTexture(tab_node->find(u"normal/1")),
      WzResource::LoadTexture(tab_node->find(u"normal/2")),
      WzResource::LoadTexture(tab_node->find(u"normal/3")),
      WzResource::LoadTexture(tab_node->find(u"normal/4")),
      WzResource::LoadTexture(tab_node->find(u"normal/5")),
  };
  for (uint8_t i = 0; i < selected_texture.size(); i++) {
    SDL_Texture* t =
        selected_tab_ == i ? selected_texture[i] : normal_texture[i];
    SDL_FPoint tab_pos = {static_cast<float>(lt.x + i * 88), lt.y};
    SDL_FRect pos_rect{static_cast<float>(int(pos_.x + tab_pos.x)),
                       static_cast<float>(int(pos_.y + tab_pos.y)),
                       static_cast<float>(t->w), static_cast<float>(t->h)};
    SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
  }
}

bool CraftUiSystem::Render() {
  RenderBackgrnd();
  RenderTab();
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

void CraftUiSystem::EventTab(SDL_Event* event) {
  const static SDL_FPoint lt = {9, 23};
  const static SDL_FPoint rb = {537, 44};
  for (uint8_t i = 0; i <= 5; i++) {
    SDL_FPoint tab_pos = {static_cast<float>(9 + i * 88), 23};
    SDL_FRect pos_rect{pos_.x + tab_pos.x, pos_.y + tab_pos.y, 88, 21};
    if (SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect)) {
      selected_tab_ = i;
    }
  }
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
          EventTab(event);
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
