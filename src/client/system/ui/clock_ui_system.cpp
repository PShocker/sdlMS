#include "clock_ui_system.h"

#include "SDL3/SDL_render.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <array>
#include <cmath>

void ClockUiSystem::RenderBackgrnd() {
  SDL_Texture *t;
  switch (type_) {
  case ClockEnum::kMin: {
    t = WzResource::LoadTexture(
        WzResource::map_->find(u"Obj/etc.img/timer/backgrnd"));
    break;
  }
  case ClockEnum::kHour: {
    t = WzResource::LoadTexture(
        WzResource::ui_->find(u"Obj/etc.img/timer/backgrndhour"));
    break;
  }
  }
  SDL_FRect pos_rect{
      pos_.x,
      pos_.y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
}

void ClockUiSystem::RenderNumber(uint32_t num, float x, float y) {
  const static std::vector<SDL_Texture *> textures = {
      WzResource::LoadTexture(
          WzResource::map_->find(u"Map/Obj/etc.img/clock/fontTime/0")),
      WzResource::LoadTexture(
          WzResource::map_->find(u"Map/Obj/etc.img/clock/fontTime/1")),
      WzResource::LoadTexture(
          WzResource::map_->find(u"Map/Obj/etc.img/clock/fontTime/2")),
      WzResource::LoadTexture(
          WzResource::map_->find(u"Map/Obj/etc.img/clock/fontTime/3")),
      WzResource::LoadTexture(
          WzResource::map_->find(u"Map/Obj/etc.img/clock/fontTime/4")),
      WzResource::LoadTexture(
          WzResource::map_->find(u"Map/Obj/etc.img/clock/fontTime/5")),
      WzResource::LoadTexture(
          WzResource::map_->find(u"Map/Obj/etc.img/clock/fontTime/6")),
      WzResource::LoadTexture(
          WzResource::map_->find(u"Map/Obj/etc.img/clock/fontTime/7")),
      WzResource::LoadTexture(
          WzResource::map_->find(u"Map/Obj/etc.img/clock/fontTime/8")),
      WzResource::LoadTexture(
          WzResource::map_->find(u"Map/Obj/etc.img/clock/fontTime/9")),
  };

  // 计算数字位数
  int digits = num == 0 ? 1 : static_cast<int>(std::log10(num)) + 1;
  // 从最高位开始遍历
  int w = 0;
  for (int i = digits - 1; i >= 0; --i) {
    int divisor = static_cast<int>(std::pow(10, i));
    int digit = (num / divisor) % 10;
    auto t = textures[digit];
    SDL_FRect pos_rect = {
        (float)x + w,
        (float)y,
        (float)t->w,
        (float)t->h,
    };
    SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
    w += t->w;
  }
}

void ClockUiSystem::RenderTime() {
  auto dt = destroy_ - Window::dt_now_;
  switch (type_) {
  case ClockEnum::kMin: {
    // render min
    auto totalSeconds = dt / 1000;    // 总秒数
    auto minutes = totalSeconds / 60; // 分钟
    auto seconds = totalSeconds % 60; // 余数秒数
    RenderNumber(minutes, pos_.x, pos_.y);

    break;
  }
  case ClockEnum::kHour: {
    auto totalSeconds = dt / 1000;             // 总秒数
    auto hours = totalSeconds / 3600;          // 小时（1小时 = 3600秒）
    auto minutes = (totalSeconds % 3600) / 60; // 分钟（除去小时后，剩余的分钟）
    RenderNumber(minutes, pos_.x, pos_.y);

    break;
  }
  }
}

bool ClockUiSystem::Render() {
  RenderBackgrnd();
  RenderTime();
  return true;
}

void ClockUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    auto &camera = CameraGameInstance::camera_;
    pos_.x = camera.w;
    pos_.y = camera.h;
    System::render_systems_.insert(it, Render);
  }
}

void ClockUiSystem::Close() { std::erase(System::render_systems_, Render); }