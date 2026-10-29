#include "src/client/system/ui/component/scroll.h"

#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>

#include <algorithm>
#include <cmath>

namespace sdlms {

void Scroll::Render() const {
  const float x = pos.x;
  const float y = pos.y;
  // 可滚动的最大首行：count - size。val 超出这个范围时（例如列表刚变短）
  // 只按最大值画，避免 thumb 跑到滚动条下界外面。
  const uint32_t max_val = count > size ? count - size : 0;
  const uint32_t cur = std::min(val, max_val);
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
  if (cur == 0) {
    // 禁用
    SDL_RenderTexture(Window::renderer_, prev, nullptr, &pos_rect);
    pre = false;
  } else {
    SDL_Texture* pv;
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
  if (cur >= max_val) {
    SDL_RenderTexture(Window::renderer_, next, nullptr, &pos_rect);
    nex = false;
  } else {
    SDL_Texture* nx;
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
  const float travel = std::max(
      0.0f, static_cast<float>(length - prev->h * 2 - thumb0->h));
  const float percent =
      max_val == 0 ? 0.0f : static_cast<float>(cur) / static_cast<float>(max_val);
  auto h = percent * travel;
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

uint32_t Scroll::Drag(SDL_FPoint mouse) const {
  if (!top) {
    return std::min(val, count > size ? count - size : 0);
  }
  // 拖动时只使用鼠标的 y，x 固定到滚动条上；y 限制在滚动条范围内。
  SDL_FPoint p = mouse;
  p.x = pos.x + 1.0f;
  p.y = std::clamp(p.y, pos.y, pos.y + static_cast<float>(length));
  return Click(p);
}

bool Scroll::InTrack(SDL_FPoint mouse) const {
  if (!top) {
    return false;
  }
  SDL_FRect track{
      pos.x,
      pos.y + 13.0f,
      15.0f,
      static_cast<float>(std::max(0, length - 26)),
  };
  return SDL_PointInRectFloat(&mouse, &track);
}

uint32_t Scroll::Click(SDL_FPoint mouse) const {
  const float x = pos.x;
  const float y = pos.y;
  // 最大首行：count - size。所有返回值都必须落在 [0, max_val] 内，
  // 否则列表会滚到下界之外（空白行 / thumb 跑出轨道）。
  const uint32_t max_val = count > size ? count - size : 0;
  const uint32_t cur = std::min(val, max_val);
  if (!top) {
    return cur;
  }
  SDL_FRect pos_rect{
      static_cast<float>((int)x),
      static_cast<float>((int)y),
      static_cast<float>(15),
      static_cast<float>(13),
  };
  if (SDL_PointInRectFloat(&mouse, &pos_rect)) {
    if (cur > 0) {
      return cur - 1;
    }
    return cur;
  }
  pos_rect = {
      static_cast<float>((int)x),
      static_cast<float>((int)y + length - 13),
      static_cast<float>(15),
      static_cast<float>(13),
  };
  if (SDL_PointInRectFloat(&mouse, &pos_rect)) {
    if (cur < max_val) {
      return cur + 1;
    }
    return cur;
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
    auto page = static_cast<uint32_t>(
        std::lround(percent * static_cast<float>(max_val)));
    return std::min(page, max_val);
  }
  return cur;
}

}  // namespace sdlms
