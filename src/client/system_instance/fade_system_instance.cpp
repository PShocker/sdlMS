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

bool fade_system_instance::render() {
  SDL_FRect pos_rect{
      0,
      0,
      (camera_game_instance::camera.w),
      (camera_game_instance::camera.h),
  };
  SDL_SetRenderDrawBlendMode(window::renderer, SDL_BLENDMODE_BLEND);
  auto alpha = mask_alpha;
  if (!fade_in) {
    alpha = 255 - alpha;
  } else {
    SDL_RenderTexture(window::renderer, backgrnd, nullptr, &pos_rect);
  }
  SDL_SetRenderDrawColor(window::renderer, 0, 0, 0, alpha);
  SDL_RenderFillRect(window::renderer, &pos_rect);
  return true;
}

void fade_system_instance::enter_in(std::function<void()> cb) {
  // save texture
  SDL_Rect pos_rect{
      0,
      0,
      static_cast<int>(camera_game_instance::camera.w),
      static_cast<int>(camera_game_instance::camera.h),
  };
  auto surface = SDL_RenderReadPixels(window::renderer, &pos_rect);
  if (backgrnd) {
    SDL_DestroyTexture(backgrnd);
    backgrnd = nullptr;
  }
  backgrnd = SDL_CreateTextureFromSurface(window::renderer, surface);
  SDL_DestroySurface(surface);

  system::logic_systems = {};
  system::render_systems = {run, render};
  system::event_systems = {keyboard_input_system::event};

  mask_alpha = 0;
  fn = cb;
  fade_in = true;
}

void fade_system_instance::enter_out() {
  system::render_systems.push_back(run);
  system::render_systems.push_back(render);

  mask_alpha = 0;
  fn = {};
  fade_in = false;

  character_logic_system::self_invincible_cooldown = UINT64_MAX;
}

bool fade_system_instance::run() {
  character_logic_system::self_invincible_cooldown = window::dt_now + 900;
  if (mask_alpha < 255) {
    mask_alpha += 15;
  } else {
    if (fn) {
      fn();
    }
    // 只移除第一个 render
    auto it = std::ranges::find(system::render_systems, &render);
    if (it != system::render_systems.end()) {
      system::render_systems.erase(it);
    }

    // 只移除第一个 run
    it = std::ranges::find(system::render_systems, &run);
    if (it != system::render_systems.end()) {
      system::render_systems.erase(it);
    }

    character_logic_system::self_invincible_cooldown = window::dt_now;
    return false;
  }
  mask_alpha = std::clamp(mask_alpha, 0, 255);
  return true;
}