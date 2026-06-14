#include "character_choose_ui_system.h"
#include "SDL3/SDL_events.h"
#include "login_ui_system.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/character_create_ui_system.h"
#include "src/client/system_instance/chatacter_create_system_instance.h"
#include "src/client/system_instance/login_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <cmath>

SDL_FPoint character_choose_ui_system::load_pos() {
  SDL_FPoint pos;
  const auto w = 1366;
  const auto h = 768;
  auto &camera = camera_game_instance::camera;
  pos.x = (camera.w - w) / 2;
  pos.y = (camera.h - h) / 2;
  return pos;
}

void character_choose_ui_system::render_button() {
  const static std::array buttons_nodes = {
      wz_resource::ui->find(u"Login.img/CharSelect/button:select"),
      wz_resource::ui->find(u"Login.img/CharSelect/button:new"),
      wz_resource::ui->find(u"Login.img/CharSelect/button:delete"),
  };
  auto pos = load_pos();
  auto &camera = camera_game_instance::camera;
  std::array buttons_rect = {
      SDL_FRect{149 - camera.x, -656 - camera.y, 129, 41},
      SDL_FRect{149 - camera.x, -607 - camera.y, 129, 45},
      SDL_FRect{151 - camera.x, -543 - camera.y, 129, 55},
  };
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

void character_choose_ui_system::render_banner() {
  static auto t = wz_resource::load_texture(
      wz_resource::ui->find(u"Login.img/LoginStart/StepBanner/CharSelect"));
  auto pos = load_pos();
  SDL_FRect pos_rect{
      pos.x + 160,
      pos.y + 85,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);

  const static std::array buttons_nodes = {
      wz_resource::ui->find(u"Login.img/LoginStart/BtClassicPrev"),
  };
  std::array buttons_rect = {
      SDL_FRect{163 + pos.x, 574 + pos.y, 161, 69},
  };
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

void character_choose_ui_system::render_backgrnd() {
  static auto t = wz_resource::load_texture(
      wz_resource::ui->find(u"Login.img/Common/classicFrame"));
  auto pos = load_pos();
  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);
}

bool character_choose_ui_system::render() {
  render_button();
  render_backgrnd();
  render_banner();
  return true;
}

void character_choose_ui_system::event_button_select() {}

void character_choose_ui_system::event_button_new() {
  system::logic_systems.push_back(new_animate);
  system::render_systems = {
      login_system_instance::render_game,
      character_create_ui_system::render,
      cursor_render_system::render,
  };
  system::event_systems = {};
}
void character_choose_ui_system::event_button_delete() {}

bool character_choose_ui_system::new_animate() {
  if (login_ui_system::camera_animate(-80, -1294)) {
    chatacter_create_system_instance::enter();
    return false;
  }
  return true;
}

bool character_choose_ui_system::back_animate() {
  if (login_ui_system::camera_animate(-80, 1023)) {
    login_system_instance::enter();
    return false;
  }
  return true;
}

void character_choose_ui_system::event_button_back() {
  system::logic_systems.push_back(back_animate);
  system::render_systems = {
      login_system_instance::render_game,
      login_ui_system::render,
      cursor_render_system::render,
  };
  system::event_systems = {};
}

bool character_choose_ui_system::event_button(SDL_Event *event) {
  std::vector<SDL_FRect> r;
  std::vector<void (*)()> fns;
  auto &camera = camera_game_instance::camera;
  auto pos = load_pos();
  r = {
      SDL_FRect{149 - camera.x, -656 - camera.y, 129, 41},
      SDL_FRect{149 - camera.x, -607 - camera.y, 129, 45},
      SDL_FRect{151 - camera.x, -543 - camera.y, 129, 55},
      SDL_FRect{163 + pos.x, 574 + pos.y, 161, 69},
  };
  fns = {
      event_button_select,
      event_button_new,
      event_button_delete,
      event_button_back,
  };
  for (size_t i = 0; i < r.size(); ++i) {
    auto pos_rect = r[i];
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    if (SDL_PointInRectFloat(&window::mouse_pos, &pos_rect)) {
      fns[i]();
      return false;
    }
  }

  return false;
}

bool character_choose_ui_system::event(SDL_Event *event) {
  bool r = true;
  switch (event->type) {
  case SDL_EVENT_KEY_DOWN: {
    auto scan_code = event->key.scancode;
    switch (scan_code) {
    case SDL_SCANCODE_ESCAPE: {
      return false;
      break;
    }
    default: {
      break;
    }
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_DOWN: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      r = false;
    }
    break;
  }
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    if (event->button.button == SDL_BUTTON_LEFT) {
      event_button(event);
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