#include "src/client/system/ui/component/slider.h"

#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include <algorithm>
#include <cmath>
#include <optional>

namespace sdlms {

namespace {

// Options.img 的 tab 背景里已经画好了滑轨。滑轨可见部分约为控件矩形内缩
// 3px 到右侧 7px；thumb 纹理左右各有 1~2px 透明边，所以再向外放 1~2px，
// 让 thumb 的黑色边框正好压住滑轨两端。
constexpr int kRailLeft = 2;
constexpr int kRailRight = 5;

SDL_FRect ToRect(int x, int y, int w, int h) {
  return SDL_FRect{static_cast<float>(x), static_cast<float>(y),
                   static_cast<float>(w), static_cast<float>(h)};
}

}  // namespace

void Slider::Render() const {
  static auto normal = WzResource::LoadTexture(
      WzResource::ui_->find(u"Slider.img/0/thumbNormal"));
  static auto pressed = WzResource::LoadTexture(
      WzResource::ui_->find(u"Slider.img/0/thumbPressed"));
  static auto disabled = WzResource::LoadTexture(
      WzResource::ui_->find(u"Slider.img/0/thumbDisabled"));
  static auto mouse_over = WzResource::LoadTexture(
      WzResource::ui_->find(u"Slider.img/0/thumbMouseOver"));

  if (normal == nullptr) {
    return;
  }

  const int track_x = pos.x + kRailLeft;
  const int track_w =
      std::max(0, length - kRailLeft - kRailRight);

  int thumb_x = 0;
  if (count <= 1) {
    // 没有有效范围时 thumb 默认居中
    thumb_x = track_x + (track_w - normal->w) / 2;
  } else {
    const uint32_t value = std::min(val, count);
    // 0 时 thumb 左边贴住滑轨左边，count 时 thumb 右边贴住滑轨右边
    const int span = std::max(0, track_w - normal->w);
    thumb_x = track_x + static_cast<int>(
                            (static_cast<int64_t>(value) * span +
                             static_cast<int64_t>(count) / 2) /
                            static_cast<int64_t>(count));
  }
  thumb_x = std::clamp(thumb_x, track_x,
                       track_x + std::max(0, track_w - normal->w));

  SDL_FRect thumb_rect = ToRect(thumb_x, pos.y, normal->w, normal->h);
  SDL_Texture* t = normal;
  if (disable) {
    t = disabled;
  } else {
    auto mouse_pos = Window::mouse_pos_;
    if (SDL_PointInRectFloat(&mouse_pos, &thumb_rect)) {
      if (Window::mouse_state_ & SDL_BUTTON_LMASK) {
        t = pressed;
      } else {
        t = mouse_over;
      }
    }
  }
  SDL_RenderTexture(Window::renderer_, t, nullptr, &thumb_rect);
}

std::optional<int> Slider::Click(SDL_FPoint mouse) const {
  SDL_FRect track_rect =
      ToRect(pos.x, pos.y, std::max(0, length), 14);
  if (!SDL_PointInRectFloat(&mouse, &track_rect)) {
    return std::nullopt;
  }
  if (count == 0) {
    return 0;
  }
  // 按可见滑轨映射，和 thumb 的左右对齐规则一致
  const int track_x = pos.x + kRailLeft;
  const int track_w = std::max(1, length - kRailLeft - kRailRight);
  float percent =
      (mouse.x - static_cast<float>(track_x)) / static_cast<float>(track_w);
  percent = std::clamp(percent, 0.0f, 1.0f);
  auto value = static_cast<int>(std::lround(percent * count));
  return std::clamp(value, 0, static_cast<int>(count));
}

}  // namespace sdlms
