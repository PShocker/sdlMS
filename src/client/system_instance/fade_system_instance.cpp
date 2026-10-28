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

bool FadeSystemInstance::render() {
  SDL_FRect pos_rect{
      0,
      0,
      (CameraGameInstance::camera.w),
      (CameraGameInstance::camera.h),
  };
  SDL_SetRenderDrawBlendMode(Window::renderer, SDL_BLENDMODE_BLEND);
  auto alpha = mask_alpha;
  if (!fade_in) {
    alpha = 255 - alpha;
  } else {
    SDL_RenderTexture(Window::renderer, backgrnd, nullptr, &pos_rect);
  }
  SDL_SetRenderDrawColor(Window::renderer, 0, 0, 0, alpha);
  SDL_RenderFillRect(Window::renderer, &pos_rect);
  return true;
}

void FadeSystemInstance::enter_in(std::function<void()> cb) {
  // save texture
  SDL_Rect pos_rect{
      0,
      0,
      static_cast<int>(CameraGameInstance::camera.w),
      static_cast<int>(CameraGameInstance::camera.h),
  };
  auto surface = SDL_RenderReadPixels(Window::renderer, &pos_rect);
  if (backgrnd) {
    SDL_DestroyTexture(backgrnd);
    backgrnd = nullptr;
  }
  backgrnd = SDL_CreateTextureFromSurface(Window::renderer, surface);
  SDL_DestroySurface(surface);

  System::logic_systems = {};
  System::render_systems = {run, render};
  System::event_systems = {KeyboardInputSystem::event};

  mask_alpha = 0;
  fn = cb;
  fade_in = true;
}

void FadeSystemInstance::enter_out() {
  System::render_systems.push_back(run);
  System::render_systems.push_back(render);

  mask_alpha = 0;
  fn = {};
  fade_in = false;

  CharacterLogicSystem::self_invincible_cooldown = UINT64_MAX;
}

bool FadeSystemInstance::run() {
  CharacterLogicSystem::self_invincible_cooldown = Window::dt_now + 900;
  if (mask_alpha < 255) {
    mask_alpha += 15;
  } else {
    if (fn) {
      fn();
    }
    // 只移除第一个 render
    auto it = std::ranges::find(System::render_systems, &render);
    if (it != System::render_systems.end()) {
      System::render_systems.erase(it);
    }

    // 只移除第一个 run
    it = std::ranges::find(System::render_systems, &run);
    if (it != System::render_systems.end()) {
      System::render_systems.erase(it);
    }

    CharacterLogicSystem::self_invincible_cooldown = Window::dt_now;
    return false;
  }
  mask_alpha = std::clamp(mask_alpha, 0, 255);
  return true;
}