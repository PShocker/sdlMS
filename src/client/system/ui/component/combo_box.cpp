#include "src/client/system/ui/component/combo_box.h"

#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include <algorithm>
#include <cmath>

namespace sdlms {

namespace {

// 下拉列表每项的高度，必须和 OptionUiSystem 的命中测试保持一致
constexpr float kItemHeight = 16.0f;
// 列表最下面留一点空白，避免和边框挤在一起
constexpr float kListBottomPad = 4.0f;

SDL_Texture* LoadBoxTexture(const char16_t* path) {
  return WzResource::LoadTexture(WzResource::ui_->find(path));
}

}  // namespace

void ComboBox::Render() const {
  const int x = static_cast<int>(rect.x);
  const int y = static_cast<int>(rect.y);

  SDL_Texture* l = LoadBoxTexture(u"Basic.img/ComboBox/normal/0");
  SDL_Texture* c = LoadBoxTexture(u"Basic.img/ComboBox/normal/1");
  SDL_Texture* r = LoadBoxTexture(u"Basic.img/ComboBox/normal/2");
  if (l == nullptr || c == nullptr || r == nullptr) {
    return;
  }
  if (disable) {
    l = LoadBoxTexture(u"Basic.img/ComboBox/disabled/0");
    c = LoadBoxTexture(u"Basic.img/ComboBox/disabled/1");
    r = LoadBoxTexture(u"Basic.img/ComboBox/disabled/2");
  } else {
    SDL_FRect pos_r{
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(w),
        static_cast<float>(l->h),
    };
    if (SDL_PointInRectFloat(&Window::mouse_pos_, &pos_r)) {
      if (Window::mouse_state_ & SDL_BUTTON_LMASK) {
        l = LoadBoxTexture(u"Basic.img/ComboBox/normal/0");
        c = LoadBoxTexture(u"Basic.img/ComboBox/normal/1");
        r = LoadBoxTexture(u"Basic.img/ComboBox/pressed/2");
      } else {
        l = LoadBoxTexture(u"Basic.img/ComboBox/normal/0");
        c = LoadBoxTexture(u"Basic.img/ComboBox/normal/1");
        r = LoadBoxTexture(u"Basic.img/ComboBox/mouseOver/2");
      }
    }
  }
  if (l == nullptr || c == nullptr || r == nullptr) {
    return;
  }

  SDL_FRect pos_rect{
      static_cast<float>(x),
      static_cast<float>(y),
      static_cast<float>(l->w),
      static_cast<float>(l->h),
  };
  SDL_RenderTexture(Window::renderer_, l, nullptr, &pos_rect);
  pos_rect.x += static_cast<float>(l->w);
  pos_rect.w = std::max(0.0f, static_cast<float>(w) - l->w - r->w);
  SDL_RenderTextureTiled(Window::renderer_, c, nullptr, 1.0f, &pos_rect);
  pos_rect.x += pos_rect.w;
  pos_rect.w = static_cast<float>(r->w);
  SDL_RenderTexture(Window::renderer_, r, nullptr, &pos_rect);

  if (!selected.empty()) {
    Freetype::LoadSize(12);
    Freetype::LoadColor(0, 0, 0, 255);
    Freetype::DrawLine(selected, static_cast<float>(x + 10),
                       static_cast<float>(y));
  }

  if (!open || selects.empty()) {
    return;
  }

  // 展开的下拉列表：用和 Options 面板一致的颜色画底、边框和选中项。
  // 列表顶边与 wz 里 combo 的矩形底边对齐，和点击命中测试保持一致。
  const float list_x = static_cast<float>(x);
  const float list_y =
      static_cast<float>(y) + std::max(rect.h, static_cast<float>(l->h));
  const float list_w = static_cast<float>(w);
  const float list_h = static_cast<float>(selects.size()) * kItemHeight;
  SDL_FRect list{list_x, list_y, list_w, list_h + kListBottomPad};
  SDL_SetRenderDrawColor(Window::renderer_, 245, 240, 230, 255);
  SDL_RenderFillRect(Window::renderer_, &list);
  SDL_SetRenderDrawColor(Window::renderer_, 60, 50, 40, 255);
  SDL_RenderRect(Window::renderer_, &list);

  Freetype::LoadSize(12);
  for (size_t i = 0; i < selects.size(); ++i) {
    SDL_FRect item{list.x + 1.0f, list.y + static_cast<float>(i) * kItemHeight,
                   list.w - 2.0f, kItemHeight};
    const bool hover = SDL_PointInRectFloat(&Window::mouse_pos_, &item);
    const bool is_selected = static_cast<int32_t>(i) == static_cast<int32_t>(val);
    if (hover || is_selected) {
      SDL_SetRenderDrawColor(Window::renderer_, hover ? 120 : 190, 170, 140,
                             255);
      SDL_RenderFillRect(Window::renderer_, &item);
    }
    Freetype::LoadColor(40, 30, 20, 255);
    Freetype::DrawStr(selects[i], item.x + 8.0f, item.y, item.w - 10.0f, 1.2f);
  }
  Freetype::LoadColor(0, 0, 0, 255);
}

}  // namespace sdlms
