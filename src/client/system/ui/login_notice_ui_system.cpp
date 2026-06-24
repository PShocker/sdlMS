#include "login_notice_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Node.h"
#include <string>
#include <vector>

SDL_FPoint login_notice_ui_system::load_pos() {
  SDL_FPoint pos;
  const auto w = 1366;
  const auto h = 768;
  auto &camera = camera_game_instance::camera;
  pos.x = (camera.w - w) / 2;
  pos.y = (camera.h - h) / 2;
  return pos;
}

void login_notice_ui_system::render_backgrnd() {
  const auto w = 1366;
  const auto h = 768;
  SDL_Texture *backgrnd;
  SDL_Texture *text;
  SDL_FPoint text_pos;
  static auto notice_node = wz_resource::ui->find(u"Login.img/Notice");
  auto [bx, by] = load_pos();
  switch (type) {
  case login_notice_system_instance::charactername_error: {
    backgrnd = wz_resource::load_texture(notice_node->find(u"backgrnd/0"));
    text =
        wz_resource::load_texture(notice_node->find(u"text/cannotUseThisName"));
    text_pos = {17, 13};
    break;
  }
  case login_notice_system_instance::charactername_used: {
    backgrnd = wz_resource::load_texture(notice_node->find(u"backgrnd/0"));
    text = wz_resource::load_texture(notice_node->find(u"text/8"));
    text_pos = {17, 13};
    break;
  }
  case login_notice_system_instance::character_delete: {
    backgrnd = wz_resource::load_texture(notice_node->find(u"backgrnd/0"));
    text = wz_resource::load_texture(notice_node->find(u"text/58"));
    text_pos = {17, 13};
    break;
  }
  case login_notice_system_instance::character_full: {
    backgrnd = wz_resource::load_texture(notice_node->find(u"backgrnd/0"));
    text = wz_resource::load_texture(
        notice_node->find(u"text/cannotCreateAccountMore"));
    text_pos = {17, 13};
    break;
  }
  case login_notice_system_instance::character_use_ap: {
    backgrnd = wz_resource::load_texture(notice_node->find(u"backgrnd/0"));
    text = wz_resource::load_texture(notice_node->find(u"text/useAllAP"));
    text_pos = {17, 13};
    break;
  }
  case login_notice_system_instance::logining: {
    backgrnd = wz_resource::load_texture(
        notice_node->find(u"Login.img/Notice/Loading/backgrnd"));
    text = nullptr;
    break;
  }
  }
  SDL_FRect pos_rect{
      bx + (w - backgrnd->w) / 2,
      by + (h - backgrnd->h) / 2,
      static_cast<float>(backgrnd->w),
      static_cast<float>(backgrnd->h),
  };
  SDL_RenderTexture(window::renderer, backgrnd, nullptr, &pos_rect);

  if (text != nullptr) {
    pos_rect.x += text_pos.x;
    pos_rect.y += text_pos.y;
    pos_rect.w = text->w;
    pos_rect.h = text->h;
    SDL_RenderTexture(window::renderer, text, nullptr, &pos_rect);
  }
  switch (type) {
  case login_notice_system_instance::logining: {
    static auto circle_node =
        wz_resource::ui->find(u"Login.img/Notice/Loading/circle");
    const auto circle_delay = 90;
    auto now = window::dt_now;
    auto index = (now / circle_delay) % (circle_node->children_count());
    auto circle_texture = circle_node->get_child(std::to_string(index));
    auto circle = wz_resource::load_texture(circle_texture);
    pos_rect.x += text_pos.x;
    pos_rect.y += text_pos.y;
    pos_rect.w = text->w;
    pos_rect.h = text->h;
    SDL_RenderTexture(window::renderer, circle, nullptr, &pos_rect);
    break;
  }
  default: {
    break;
  }
  }
}

void login_notice_ui_system::render_button() {
  const auto w = 1366;
  const auto h = 768;
  std::vector<wz::Node *> buttons_nodes;
  std::vector<SDL_FRect> buttons_rect;
  auto [bx, by] = load_pos();
  switch (type) {
  case login_notice_system_instance::charactername_error:
  case login_notice_system_instance::character_full:
  case login_notice_system_instance::character_use_ap:
  case login_notice_system_instance::charactername_used: {
    buttons_nodes = {
        wz_resource::ui->find(u"Login.img/Notice/BtYes"),
    };
    buttons_rect = {
        SDL_FRect{bx + (w - 50) / 2, by + 35 + (h) / 2, 50, 23},
    };
    break;
  }
  case login_notice_system_instance::character_delete: {
    buttons_nodes = {
        wz_resource::ui->find(u"Login.img/Notice/BtYes"),
        wz_resource::ui->find(u"Login.img/Notice/BtNo"),
    };
    buttons_rect = {
        SDL_FRect{bx - 25 + (w - 50) / 2, by + 35 + (h) / 2, 50, 23},
        SDL_FRect{bx + 25 + (w - 50) / 2, by + 35 + (h) / 2, 50, 23},
    };
    break;
  }
  case login_notice_system_instance::logining: {
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
  const auto w = 1366;
  const auto h = 768;
  std::vector<SDL_FRect> buttons_rect;
  std::vector<void (*)()> fns = {event_close};
  auto [bx, by] = load_pos();
  switch (type) {
  case login_notice_system_instance::charactername_error:
  case login_notice_system_instance::character_full:
  case login_notice_system_instance::character_use_ap:
  case login_notice_system_instance::charactername_used: {
    buttons_rect = {
        SDL_FRect{bx + (w - 50) / 2, by + 35 + (h) / 2, 50, 23},
    };
    break;
  }
  case login_notice_system_instance::character_delete: {
    buttons_rect = {
        SDL_FRect{bx - 25 + (w - 50) / 2, by + 35 + (h) / 2, 50, 23},
        SDL_FRect{bx + 25 + (w - 50) / 2, by + 35 + (h) / 2, 50, 23},
    };
    fns = {fn, event_close};
    break;
  }
  default: {
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

  return false;
}

bool login_notice_ui_system::run() { return true; }