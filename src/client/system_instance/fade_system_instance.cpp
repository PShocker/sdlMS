#include "fade_system_instance.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/system/input/keyboard_input_system.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include <algorithm>
#include <cstdint>
#include <ranges>

bool FadeSystemInstance::Render() {
  SDL_FRect pos_rect{
      0,
      0,
      (CameraGameInstance::camera_.w),
      (CameraGameInstance::camera_.h),
  };
  SDL_SetRenderDrawBlendMode(Window::renderer_, SDL_BLENDMODE_BLEND);
  auto alpha = mask_alpha_;
  if (!fade_in_) {
    alpha = 255 - alpha;
  } else {
    SDL_RenderTexture(Window::renderer_, backgrnd_, nullptr, &pos_rect);
  }
  SDL_SetRenderDrawColor(Window::renderer_, 0, 0, 0, alpha);
  SDL_RenderFillRect(Window::renderer_, &pos_rect);
  return true;
}

void FadeSystemInstance::EnterIn(std::function<void()> cb) {
  // save texture
  SDL_Rect pos_rect{
      0,
      0,
      static_cast<int>(CameraGameInstance::camera_.w),
      static_cast<int>(CameraGameInstance::camera_.h),
  };
  auto surface = SDL_RenderReadPixels(Window::renderer_, &pos_rect);
  if (backgrnd_) {
    SDL_DestroyTexture(backgrnd_);
    backgrnd_ = nullptr;
  }
  backgrnd_ = SDL_CreateTextureFromSurface(Window::renderer_, surface);
  SDL_DestroySurface(surface);

  System::logic_systems_ = {};
  System::render_systems_ = {Run, Render};
  System::event_systems_ = {KeyboardInputSystem::Event};

  mask_alpha_ = 0;
  fn_ = cb;
  fade_in_ = true;
}

void FadeSystemInstance::EnterOut() {
  System::render_systems_.push_back(Run);
  System::render_systems_.push_back(Render);

  mask_alpha_ = 0;
  fn_ = {};
  fade_in_ = false;

  CharacterLogicSystem::self_invincible_cooldown_ = UINT64_MAX;
}

bool FadeSystemInstance::Run() {
  CharacterLogicSystem::self_invincible_cooldown_ = Window::dt_now_ + 900;
  if (mask_alpha_ < 255) {
    mask_alpha_ += 15;
  } else {
    if (fn_) {
      fn_();
    }
    // 只移除第一个 render
    auto it = std::ranges::find(System::render_systems_, &Render);
    if (it != System::render_systems_.end()) {
      System::render_systems_.erase(it);
    }

    // 只移除第一个 run
    it = std::ranges::find(System::render_systems_, &Run);
    if (it != System::render_systems_.end()) {
      System::render_systems_.erase(it);
    }

    CharacterLogicSystem::self_invincible_cooldown_ = Window::dt_now_;
    return false;
  }
  mask_alpha_ = std::clamp(mask_alpha_, 0, 255);
  return true;
}