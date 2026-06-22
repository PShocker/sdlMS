#include "login_notice_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include <vector>

SDL_FPoint login_notice_ui_system::load_pos() { return {100, 100}; }

void login_notice_ui_system::render_backgrnd() {
  SDL_Texture *backgrnd;
  SDL_Texture *text;
  SDL_FPoint text_pos;
  static auto notice_node = wz_resource::ui->find(u"Login.img/Notice");
  auto [bx, by] = load_pos();
  switch (type) {
  case login_notice_system_instance::charactername_error: {
    backgrnd = wz_resource::load_texture(notice_node->find(u"backgrnd/0"));
    text = wz_resource::load_texture(notice_node->find(u"text/28"));
    break;
  }
  }
  SDL_FRect pos_rect{
      bx,
      by,
      static_cast<float>(backgrnd->w),
      static_cast<float>(backgrnd->h),
  };
  SDL_RenderTexture(window::renderer, backgrnd, nullptr, &pos_rect);

  pos_rect.x += text_pos.x;
  pos_rect.y += text_pos.y;
  pos_rect.w = text->w;
  pos_rect.h = text->h;
  SDL_RenderTexture(window::renderer, text, nullptr, &pos_rect);
}

void login_notice_ui_system::render_button() {
  std::vector<wz::Node *> buttons_nodes;
  std::vector<SDL_FRect> buttons_rect;
  auto [bx, by] = load_pos();
  switch (type) {
  case login_notice_system_instance::charactername_error: {
    buttons_nodes = {
        wz_resource::ui->find(u"Login.img/Notice/BtYes"),
    };
    buttons_rect = {SDL_FRect{bx, by, 50, 23}};
    break;
  }
  }
  for (size_t i = 0; i < buttons_nodes.size(); ++i) {
    auto k = buttons_nodes[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    auto &mouse_pos = window::mouse_pos;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
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

bool login_notice_ui_system::render() {
  render_backgrnd();
  render_button();
  return true;
}

void login_notice_ui_system::event_close() {
  std::erase(system::render_systems, render);
  std::erase(system::event_systems, event);
}

bool login_notice_ui_system::event_button(SDL_Event *event) {
  std::vector<SDL_FRect> buttons_rect;
  std::vector<void (*)()> fns;
  auto [bx, by] = load_pos();
  switch (type) {
  case login_notice_system_instance::charactername_error: {
    buttons_rect = {SDL_FRect{bx, by, 50, 23}};
    fns = {fn};
    break;
  }
  }
  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      fns[i]();
      return false;
    }
  }
  return true;
}

bool login_notice_ui_system::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      r = event_button(event);
    }
    break;
  }
  case SDL_EVENT_MOUSE_MOTION: {
    break;
  }
  default: {
    break;
  }
  }

  return r;
}