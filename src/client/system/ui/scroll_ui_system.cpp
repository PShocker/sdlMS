#include "scroll_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <algorithm>
#include <cmath>

void scroll_ui_system::render_vscroll(float x, float y, uint32_t val,
                                      uint32_t count, int32_t length,
                                      bool top) {
  static auto prev0 = wz_resource::load_texture(
      wz_resource::ui->find(u"Basic.img/VScr4/enabled/prev0"));
  static auto prev1 = wz_resource::load_texture(
      wz_resource::ui->find(u"Basic.img/VScr4/enabled/prev1"));
  static auto prev2 = wz_resource::load_texture(
      wz_resource::ui->find(u"Basic.img/VScr4/enabled/prev2"));
  static auto next0 = wz_resource::load_texture(
      wz_resource::ui->find(u"Basic.img/VScr4/enabled/next0"));
  static auto next1 = wz_resource::load_texture(
      wz_resource::ui->find(u"Basic.img/VScr4/enabled/next1"));
  static auto next2 = wz_resource::load_texture(
      wz_resource::ui->find(u"Basic.img/VScr4/enabled/next2"));
  static auto base0 = wz_resource::load_texture(
      wz_resource::ui->find(u"Basic.img/VScr4/enabled/base"));
  static auto thumb0 = wz_resource::load_texture(
      wz_resource::ui->find(u"Basic.img/VScr4/enabled/thumb0"));
  static auto thumb1 = wz_resource::load_texture(
      wz_resource::ui->find(u"Basic.img/VScr4/enabled/thumb1"));
  static auto thumb2 = wz_resource::load_texture(
      wz_resource::ui->find(u"Basic.img/VScr4/enabled/thumb2"));
  static auto prev = wz_resource::load_texture(
      wz_resource::ui->find(u"Basic.img/VScr4/disabled/prev"));
  static auto next = wz_resource::load_texture(
      wz_resource::ui->find(u"Basic.img/VScr4/disabled/next"));
  static auto base = wz_resource::load_texture(
      wz_resource::ui->find(u"Basic.img/VScr4/disabled/base"));
  bool pre = true;
  bool nex = true;
  // 判断上按钮是否禁用
  SDL_FRect pos_rect{static_cast<float>((int)x), static_cast<float>((int)y),
                     static_cast<float>(prev->w), static_cast<float>(prev->h)};
  if (val <= 0) {
    // 禁用
    SDL_RenderTexture(window::renderer, prev, nullptr, &pos_rect);
    pre = false;
  } else {
    SDL_Texture *pv;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect) && top) {
      if (window::mouse_state & SDL_BUTTON_LMASK) {
        pv = prev1;
      } else {
        pv = prev2;
      }
    } else {
      pv = prev0;
    }
    SDL_RenderTexture(window::renderer, pv, nullptr, &pos_rect);
  }

  // 下按钮
  pos_rect = {static_cast<float>((int)x),
              static_cast<float>((int)y + length - next->h),
              static_cast<float>(next->w), static_cast<float>(next->h)};
  if (val >= count) {
    SDL_RenderTexture(window::renderer, next, nullptr, &pos_rect);
    nex = false;
  } else {
    SDL_Texture *nx;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect) && top) {
      if (window::mouse_state & SDL_BUTTON_LMASK) {
        nx = next1;
      } else {
        nx = next2;
      }
    } else {
      nx = next0;
    }
    SDL_RenderTexture(window::renderer, nx, nullptr, &pos_rect);
  }

  // 滚动条
  pos_rect = {x, y + prev->h, static_cast<float>(base->w),
              static_cast<float>(length - prev->h - next->h)};
  if (pre == false && nex == false) {
    SDL_RenderTextureTiled(window::renderer, base, nullptr, 1, &pos_rect);
  } else {
    SDL_RenderTextureTiled(window::renderer, base0, nullptr, 1, &pos_rect);
  }

  // thumb
  float percent = (float)val / count;
  auto h = percent * (length - prev->h * 2) - thumb0->h / 2;
  h = std::clamp(h, 0.0f, float(length - prev->h * 2 - thumb0->h));
  pos_rect = {static_cast<float>((int)x),
              static_cast<float>(int(y + prev->h + h)),
              static_cast<float>(thumb0->w), static_cast<float>(thumb0->h)};
  if (pre == true || nex == true) {
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect) && top) {
      SDL_RenderTexture(window::renderer, thumb2, nullptr, &pos_rect);
    } else {
      SDL_RenderTexture(window::renderer, thumb0, nullptr, &pos_rect);
    }
  }
}

uint32_t scroll_ui_system::click_vscroll(float x, float y, uint32_t val,
                                         uint32_t count, int32_t length,
                                         bool top) {
  if (!top) {
    return val;
  }
  auto mouse = window::mouse_pos;
  SDL_FRect pos_rect{static_cast<float>((int)x), static_cast<float>((int)y),
                     static_cast<float>(15), static_cast<float>(13)};
  if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
    if (val > 0) {
      val -= 1;
      return val;
    }
  }
  pos_rect = {static_cast<float>((int)x),
              static_cast<float>((int)y + length - 13), static_cast<float>(15),
              static_cast<float>(13)};
  if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
    if (val < count) {
      val += 1;
      return val;
    }
  }
  pos_rect = {x, y + 13, static_cast<float>(15),
              static_cast<float>(length - 26)};
  if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
    auto dy = window::mouse_pos.y - pos_rect.y;
    float percent = (float)dy / pos_rect.h;
    val = std::round(percent * count);
    return val;
  }
  return val;
}