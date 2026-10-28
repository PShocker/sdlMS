#include "beauty_ui_system.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <algorithm>
namespace sdlms {


void BeautyUiSystem::RenderBackgrnd() {
  static auto backgrnd = WzResource::LoadTexture(
      WzResource::ui_->find(u"Login.img/NewChar/customize/canvas:top"));
  SDL_FRect pos_rect = {
      160,
      85,
      static_cast<float>(backgrnd->w),
      static_cast<float>(backgrnd->h),
  };
  SDL_RenderTexture(Window::renderer_, backgrnd, nullptr, &pos_rect);
}

void BeautyUiSystem::RenderButton() {}

SDL_FPoint BeautyUiSystem::LoadWh() { return {545, 409}; }

bool BeautyUiSystem::Render() { return true; }

void BeautyUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    auto wh = LoadWh();
    auto &camera = CameraGameInstance::camera_;
    pos_.x = (camera.w - wh.x) / 2;
    pos_.y = (camera.h - wh.y) / 2;

    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

void BeautyUiSystem::Close() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
}

void BeautyUiSystem::Toggle() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
  auto fn = &Render;
  if (std::ranges::contains(System::render_systems_, fn)) {
    Close();
  } else {
    Open();
  }
}

bool BeautyUiSystem::CursorIn() {
  auto [w, h] = LoadWh();
  auto &mouse = Window::mouse_pos_;
  SDL_FRect pos_rect{pos_.x, pos_.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

bool BeautyUiSystem::Event(SDL_Event *Event) {
  bool r = true;
  switch (Event->type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (Event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == Render) {
        r = false;
      }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (Event->button.button == SDL_BUTTON_LEFT) {
      if (CursorGameInstance::cursor_ui == Render) {
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
