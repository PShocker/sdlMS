#include "screen_notice_ui_system.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <ranges>

SDL_FPoint screen_notice_ui_system::load_wh() { return {209, 289}; }

void screen_notice_ui_system::render_backgrnd() {
  auto [w, h] = load_wh();
  auto n = wz_resource::ui->find(u"UIWindow.img/FloatNotice/" + path);
  auto l = wz_resource::load_texture(n->get_child(u"0"));
  auto m = wz_resource::load_texture(n->get_child(u"1"));
  auto r = wz_resource::load_texture(n->get_child(u"2"));
  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(l->w),
      static_cast<float>(l->h),
  };
  SDL_RenderTexture(window::renderer, l, nullptr, &pos_rect);
  pos_rect = {
      pos.x + l->w,
      pos.y,
      static_cast<float>(w - l->w - r->w),
      static_cast<float>(m->h),
  };
  SDL_RenderTextureTiled(window::renderer, m, nullptr, 1, &pos_rect);
  pos_rect = {
      pos.x,
      pos.y,
      static_cast<float>(r->w),
      static_cast<float>(r->h),
  };
  SDL_RenderTexture(window::renderer, r, nullptr, &pos_rect);
}

void screen_notice_ui_system::open() {
  auto it =
      std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    auto wh = load_wh();
    auto &camera = camera_game_instance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    system::render_systems.insert(it, render);
    system::event_systems.insert(system::event_systems.begin(), event);
  }
}

bool screen_notice_ui_system::render() { return true; }

bool screen_notice_ui_system::event(SDL_Event *event) { return true; }

void screen_notice_ui_system::close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
}