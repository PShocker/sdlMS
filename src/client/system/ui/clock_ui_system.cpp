#include "clock_ui_system.h"

#include "SDL3/SDL_render.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <array>
#include <cmath>

void clock_ui_system::render_backgrnd() {
  SDL_Texture *t;
  switch (type) {
  case clock_enum::min: {
    t = wz_resource::load_texture(
        wz_resource::map->find(u"Obj/etc.img/timer/backgrnd"));
    break;
  }
  case clock_enum::hour: {
    t = wz_resource::load_texture(
        wz_resource::ui->find(u"Obj/etc.img/timer/backgrndhour"));
    break;
  }
  }
  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
}

void clock_ui_system::render_number(uint32_t num, float x, float y) {
  const static std::vector<SDL_Texture *> textures = {
      wz_resource::load_texture(
          wz_resource::map->find(u"Map/Obj/etc.img/clock/fontTime/0")),
      wz_resource::load_texture(
          wz_resource::map->find(u"Map/Obj/etc.img/clock/fontTime/1")),
      wz_resource::load_texture(
          wz_resource::map->find(u"Map/Obj/etc.img/clock/fontTime/2")),
      wz_resource::load_texture(
          wz_resource::map->find(u"Map/Obj/etc.img/clock/fontTime/3")),
      wz_resource::load_texture(
          wz_resource::map->find(u"Map/Obj/etc.img/clock/fontTime/4")),
      wz_resource::load_texture(
          wz_resource::map->find(u"Map/Obj/etc.img/clock/fontTime/5")),
      wz_resource::load_texture(
          wz_resource::map->find(u"Map/Obj/etc.img/clock/fontTime/6")),
      wz_resource::load_texture(
          wz_resource::map->find(u"Map/Obj/etc.img/clock/fontTime/7")),
      wz_resource::load_texture(
          wz_resource::map->find(u"Map/Obj/etc.img/clock/fontTime/8")),
      wz_resource::load_texture(
          wz_resource::map->find(u"Map/Obj/etc.img/clock/fontTime/9")),
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
    SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
    w += t->w;
  }
}

void clock_ui_system::render_time() {
  auto dt = destroy‌ - window::dt_now;
  switch (type) {
  case clock_enum::min: {
    // render min
    auto totalSeconds = dt / 1000;    // 总秒数
    auto minutes = totalSeconds / 60; // 分钟
    auto seconds = totalSeconds % 60; // 余数秒数
    render_number(minutes, pos.x, pos.y);
    
    break;
  }
  case clock_enum::hour: {
    auto totalSeconds = dt / 1000;             // 总秒数
    auto hours = totalSeconds / 3600;          // 小时（1小时 = 3600秒）
    auto minutes = (totalSeconds % 3600) / 60; // 分钟（除去小时后，剩余的分钟）
    render_number(minutes, pos.x, pos.y);

    break;
  }
  }
}

bool clock_ui_system::render() {
  render_backgrnd();
  render_time();
  return true;
}

void clock_ui_system::open() {
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    auto &camera = camera_game_instance::camera;
    pos.x = camera.w;
    pos.y = camera.h;
    system::render_systems.insert(it, render);
  }
}

void clock_ui_system::close() { std::erase(system::render_systems, render); }