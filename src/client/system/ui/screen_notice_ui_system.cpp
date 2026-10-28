#include "screen_notice_ui_system.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include <ranges>

SDL_FPoint ScreenNoticeUiSystem::LoadWh() { return {209, 289}; }

void ScreenNoticeUiSystem::RenderBackgrnd() {
  auto [w, h] = LoadWh();
  auto n = WzResource::ui_->find(u"UIWindow.img/FloatNotice/" + path_);
  auto l = WzResource::LoadTexture(n->get_child(u"0"));
  auto m = WzResource::LoadTexture(n->get_child(u"1"));
  auto r = WzResource::LoadTexture(n->get_child(u"2"));
  SDL_FRect pos_rect{
      pos_.x,
      pos_.y,
      static_cast<float>(l->w),
      static_cast<float>(l->h),
  };
  SDL_RenderTexture(Window::renderer_, l, nullptr, &pos_rect);
  pos_rect = {
      pos_.x + l->w,
      pos_.y,
      static_cast<float>(w - l->w - r->w),
      static_cast<float>(m->h),
  };
  SDL_RenderTextureTiled(Window::renderer_, m, nullptr, 1, &pos_rect);
  pos_rect = {
      pos_.x,
      pos_.y,
      static_cast<float>(r->w),
      static_cast<float>(r->h),
  };
  SDL_RenderTexture(Window::renderer_, r, nullptr, &pos_rect);
}

void ScreenNoticeUiSystem::RenderText() {
  Freetype::LoadSize(12);
  Freetype::LoadAligned(true);
  Freetype::LoadColor(255, 255, 255, 255);
  // freetype::draw_str();
}

void ScreenNoticeUiSystem::Open() {
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

bool ScreenNoticeUiSystem::Render() {
  RenderBackgrnd();

  return true;
}

bool ScreenNoticeUiSystem::Event(SDL_Event *Event) { return true; }

void ScreenNoticeUiSystem::Close() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
}