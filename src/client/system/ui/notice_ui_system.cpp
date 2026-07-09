#include "notice_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include <string>

void notice_ui_system::render_backgrnd() {
  wz::Node *node;
  switch (type) {
  case notice_enum::shopbuy: {
    node = wz_resource::ui->find(u"PopupWindow.img/Notice1");
    break;
  }
  case notice_enum::shopbuy_mul: {
    node = wz_resource::ui->find(u"PopupWindow.img/Notice2");
    break;
  }
  }
  auto [w, h] = load_wh();
  static auto t = wz_resource::load_texture(node->get_child(u"t"));
  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
  static auto c = wz_resource::load_texture(node->get_child(u"c"));
  static auto s = wz_resource::load_texture(node->get_child(u"s"));
  pos_rect = {
      pos.x,
      pos.y + t->h,
      static_cast<float>(c->w),
      static_cast<float>(h - t->h - s->h),
  };
  SDL_RenderTextureTiled(window::renderer, c, nullptr, 1, &pos_rect);

  pos_rect = {
      pos.x,
      pos.y + h - s->h,
      static_cast<float>(s->w),
      static_cast<float>(s->h),
  };
  SDL_RenderTexture(window::renderer, s, nullptr, &pos_rect);
}

void notice_ui_system::render_button() {
  std::vector<wz::Node *> buttons_node = {};
  std::vector<SDL_FRect> buttons_rect = {};
  switch (type) {
  case notice_enum::shopbuy: {
    buttons_node = {};
    buttons_rect = {};
    break;
  }
  case notice_enum::shopbuy_mul: {
    buttons_node = {};
    buttons_rect = {};
    break;
  }
  }
  for (size_t i = 0; i < buttons_node.size(); ++i) {
    auto k = buttons_node[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x += (int)pos.x;
    pos_rect.y += (int)pos.y;
    auto &mouse_pos = window::mouse_pos;
    // 判断按钮是否被遮挡
    auto cursor_in = cursor_game_instance::cursor_ui;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && cursor_in == render) {
      if (window::mouse_state & SDL_BUTTON_LMASK) {
        auto pressed = wz_resource::load_texture(k->find(u"pressed/0"));
        SDL_RenderTexture(window::renderer, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = wz_resource::load_texture(k->find(u"mouseOver/0"));
        SDL_RenderTexture(window::renderer, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = wz_resource::load_texture(k->find(u"normal/0"));
      SDL_RenderTexture(window::renderer, normal, nullptr, &pos_rect);
    }
  }
}

void notice_ui_system::render_text() {
  std::u16string text = u"";
  SDL_FPoint p;
  switch (type) {
  case notice_enum::shopbuy: {
    break;
  }
  case notice_enum::shopbuy_mul: {
    break;
  }
  }
  if (!text.empty()) {
  }
}

bool notice_ui_system::render() {
  render_backgrnd();
  render_button();
  return true;
}

SDL_FPoint notice_ui_system::load_wh() {
  switch (type) {
  case notice_enum::shopbuy: {
    return {266, 96};
  }
  case notice_enum::shopbuy_mul:
    return {266, 119};
  }
  return {0, 0};
}
