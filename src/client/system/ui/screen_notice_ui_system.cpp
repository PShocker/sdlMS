#include "screen_notice_ui_system.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include <ranges>

SDL_FPoint ScreenNoticeUiSystem::load_wh() { return {209, 289}; }

void ScreenNoticeUiSystem::render_backgrnd() {
  auto [w, h] = load_wh();
  auto n = WzResource::ui->find(u"UIWindow.img/FloatNotice/" + path);
  auto l = WzResource::load_texture(n->get_child(u"0"));
  auto m = WzResource::load_texture(n->get_child(u"1"));
  auto r = WzResource::load_texture(n->get_child(u"2"));
  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(l->w),
      static_cast<float>(l->h),
  };
  SDL_RenderTexture(Window::renderer, l, nullptr, &pos_rect);
  pos_rect = {
      pos.x + l->w,
      pos.y,
      static_cast<float>(w - l->w - r->w),
      static_cast<float>(m->h),
  };
  SDL_RenderTextureTiled(Window::renderer, m, nullptr, 1, &pos_rect);
  pos_rect = {
      pos.x,
      pos.y,
      static_cast<float>(r->w),
      static_cast<float>(r->h),
  };
  SDL_RenderTexture(Window::renderer, r, nullptr, &pos_rect);
}

void ScreenNoticeUiSystem::render_text() {
  Freetype::load_size(12);
  Freetype::load_aligned(true);
  Freetype::load_color(255, 255, 255, 255);
  // freetype::draw_str();
}

void ScreenNoticeUiSystem::open() {
  auto it =
      std::ranges::find(System::render_systems, &CursorRenderSystem::render);
  if (it != System::render_systems.end()) {
    auto wh = load_wh();
    auto &camera = CameraGameInstance::camera;
    pos.x = (camera.w - wh.x) / 2;
    pos.y = (camera.h - wh.y) / 2;

    System::render_systems.insert(it, render);
    System::event_systems.insert(System::event_systems.begin(), event);
  }
}

bool ScreenNoticeUiSystem::render() {
  render_backgrnd();

  return true;
}

bool ScreenNoticeUiSystem::event(SDL_Event *event) { return true; }

void ScreenNoticeUiSystem::close() {
  std::erase(System::render_systems, render);
  std::erase(System::event_systems, event);
}