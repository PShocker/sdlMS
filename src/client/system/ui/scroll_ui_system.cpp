#include "src/client/system/ui/scroll_ui_system.h"
#include <SDL3/SDL_rect.h>
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <algorithm>
#include <cmath>
namespace sdlms {


void ScrollUiSystem::RenderVscroll(float x, float y, uint32_t val,
                                      uint32_t count, int32_t length, bool top,
                                      uint32_t size) {
  static auto prev0 = WzResource::LoadTexture(
      WzResource::ui_->find(u"Basic.img/VScr4/enabled/prev0"));
  static auto prev1 = WzResource::LoadTexture(
      WzResource::ui_->find(u"Basic.img/VScr4/enabled/prev1"));
  static auto prev2 = WzResource::LoadTexture(
      WzResource::ui_->find(u"Basic.img/VScr4/enabled/prev2"));
  static auto next0 = WzResource::LoadTexture(
      WzResource::ui_->find(u"Basic.img/VScr4/enabled/next0"));
  static auto next1 = WzResource::LoadTexture(
      WzResource::ui_->find(u"Basic.img/VScr4/enabled/next1"));
  static auto next2 = WzResource::LoadTexture(
      WzResource::ui_->find(u"Basic.img/VScr4/enabled/next2"));
  static auto base0 = WzResource::LoadTexture(
      WzResource::ui_->find(u"Basic.img/VScr4/enabled/base"));
  static auto thumb0 = WzResource::LoadTexture(
      WzResource::ui_->find(u"Basic.img/VScr4/enabled/thumb0"));
  static auto thumb1 = WzResource::LoadTexture(
      WzResource::ui_->find(u"Basic.img/VScr4/enabled/thumb1"));
  static auto thumb2 = WzResource::LoadTexture(
      WzResource::ui_->find(u"Basic.img/VScr4/enabled/thumb2"));
  static auto prev = WzResource::LoadTexture(
      WzResource::ui_->find(u"Basic.img/VScr4/disabled/prev"));
  static auto next = WzResource::LoadTexture(
      WzResource::ui_->find(u"Basic.img/VScr4/disabled/next"));
  static auto base = WzResource::LoadTexture(
      WzResource::ui_->find(u"Basic.img/VScr4/disabled/base"));
  bool pre = true;
  bool nex = true;
  // 判断上按钮是否禁用
  SDL_FRect pos_rect{static_cast<float>((int)x), static_cast<float>((int)y),
                     static_cast<float>(prev->w), static_cast<float>(prev->h)};
  if (val <= 0) {
    // 禁用
    SDL_RenderTexture(Window::renderer_, prev, nullptr, &pos_rect);
    pre = false;
  } else {
    SDL_Texture *pv;
    if (SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect) && top) {
      if (Window::mouse_state_ & SDL_BUTTON_LMASK) {
        pv = prev1;
      } else {
        pv = prev2;
      }
    } else {
      pv = prev0;
    }
    SDL_RenderTexture(Window::renderer_, pv, nullptr, &pos_rect);
  }

  // 下按钮
  pos_rect = {static_cast<float>((int)x),
              static_cast<float>((int)y + length - next->h),
              static_cast<float>(next->w), static_cast<float>(next->h)};
  if (val >= count || val + size >= count) {
    SDL_RenderTexture(Window::renderer_, next, nullptr, &pos_rect);
    nex = false;
  } else {
    SDL_Texture *nx;
    if (SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect) && top) {
      if (Window::mouse_state_ & SDL_BUTTON_LMASK) {
        nx = next1;
      } else {
        nx = next2;
      }
    } else {
      nx = next0;
    }
    SDL_RenderTexture(Window::renderer_, nx, nullptr, &pos_rect);
  }
  // 滚动条
  pos_rect = {x, y + prev->h, static_cast<float>(base->w),
              static_cast<float>(length - prev->h - next->h)};
  if (pre == false && nex == false) {
    SDL_RenderTextureTiled(Window::renderer_, base, nullptr, 1, &pos_rect);
  } else {
    SDL_RenderTextureTiled(Window::renderer_, base0, nullptr, 1, &pos_rect);
    if (top && SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect)) {
      if (!CursorGameInstance::cursor_vscr_) {
        CursorGameInstance::cursor_vscr_ = true;
      }
    }
  }

  // thumb
  float percent = (float)val / (count - size);
  // percent = 1;
  auto h = percent * (length - prev->h * 2 - thumb0->h);
  pos_rect = {static_cast<float>((int)x),
              static_cast<float>(int(y + prev->h + h)),
              static_cast<float>(thumb0->w), static_cast<float>(thumb0->h)};
  if (pre == true || nex == true) {
    if (SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect) && top) {
      SDL_RenderTexture(Window::renderer_, thumb2, nullptr, &pos_rect);
    } else {
      SDL_RenderTexture(Window::renderer_, thumb0, nullptr, &pos_rect);
    }
  }
}

uint32_t ScrollUiSystem::ClickVscroll(float x, float y, uint32_t val,
                                         uint32_t count, int32_t length,
                                         bool top, SDL_FPoint mouse) {
  if (!top) {
    return val;
  }
  SDL_FRect pos_rect{
      static_cast<float>((int)x),
      static_cast<float>((int)y),
      static_cast<float>(15),
      static_cast<float>(13),
  };
  if (SDL_PointInRectFloat(&mouse, &pos_rect)) {
    if (val > 0) {
      val -= 1;
      return val;
    }
  }
  pos_rect = {
      static_cast<float>((int)x),
      static_cast<float>((int)y + length - 13),
      static_cast<float>(15),
      static_cast<float>(13),
  };
  if (SDL_PointInRectFloat(&mouse, &pos_rect)) {
    if (val < count) {
      val += 1;
      return val;
    }
  }
  pos_rect = {
      x,
      y + 13,
      static_cast<float>(15),
      static_cast<float>(length - 26),
  };
  if (SDL_PointInRectFloat(&mouse, &pos_rect)) {
    auto dy = mouse.y - pos_rect.y;
    float percent = (float)dy / pos_rect.h;
    if (percent <= 0.02) {
      percent = 0;
    } else if (percent >= 0.98) {
      percent = 1;
    }
    val = std::round(percent * count);
    return val;
  }
  return val;
}

bool ScrollUiSystem::ClickThumb(float x, float y, int32_t length,
                                   SDL_FPoint mouse) {
  SDL_FRect pos_rect{
      x,
      y + 13,
      static_cast<float>(15),
      static_cast<float>(length - 26),
  };
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

}  // namespace sdlms
