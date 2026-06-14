#include "character_create_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "login_ui_system.h"
#include "src/client/game/game_character.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/render/character_render_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/character_choose_ui_system.h"
#include "src/client/system_instance/character_choose_system_instance.h"
#include "src/client/system_instance/login_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include <cmath>

void character_create_ui_system::render_button() {
  const static std::array buttons_nodes = {
      wz_resource::ui->find(u"Login.img/NewChar/button:ok"),
      wz_resource::ui->find(u"Login.img/NewChar/button:cancel"),
  };
  auto pos = load_pos();
  auto &camera = camera_game_instance::camera;
  std::array buttons_rect = {
      SDL_FRect{66 - camera.x, -1154 - camera.y, 104, 52},
      SDL_FRect{160 - camera.x, -1156 - camera.y, 104, 52},
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

SDL_FPoint character_create_ui_system::load_pos() {
  SDL_FPoint pos;
  const auto w = 1366;
  const auto h = 768;
  auto &camera = camera_game_instance::camera;
  pos.x = (camera.w - w) / 2;
  pos.y = (camera.h - h) / 2;
  return pos;
}

void character_create_ui_system::render_backgrnd() {
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

void character_create_ui_system::render_stat() {
  static auto t = wz_resource::load_texture(
      wz_resource::ui->find(u"Login.img/NewChar/stat/canvas:statback"));
  auto &camera = camera_game_instance::camera;
  SDL_FRect pos_rect{
      70 - camera.x,
      -1322 - camera.y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(window::renderer, t, nullptr, &pos_rect);

  const static std::array buttons_nodes = {
      wz_resource::ui->find(u"Login.img/NewChar/stat/button:str_prev"),
      wz_resource::ui->find(u"Login.img/NewChar/stat/button:str_next"),
      wz_resource::ui->find(u"Login.img/NewChar/stat/button:dex_prev"),
      wz_resource::ui->find(u"Login.img/NewChar/stat/button:dex_next"),
      wz_resource::ui->find(u"Login.img/NewChar/stat/button:int_prev"),
      wz_resource::ui->find(u"Login.img/NewChar/stat/button:int_next"),
      wz_resource::ui->find(u"Login.img/NewChar/stat/button:luk_prev"),
      wz_resource::ui->find(u"Login.img/NewChar/stat/button:luk_next"),
  };
  std::array buttons_rect = {
      SDL_FRect{pos_rect.x + 48, pos_rect.y + 4, 16, 17},
      SDL_FRect{pos_rect.x + 96, pos_rect.y + 4, 16, 17},
      SDL_FRect{pos_rect.x + 48, pos_rect.y + 27, 16, 17},
      SDL_FRect{pos_rect.x + 96, pos_rect.y + 27, 16, 17},
      SDL_FRect{pos_rect.x + 48, pos_rect.y + 50, 16, 17},
      SDL_FRect{pos_rect.x + 96, pos_rect.y + 50, 16, 17},
      SDL_FRect{pos_rect.x + 48, pos_rect.y + 73, 16, 17},
      SDL_FRect{pos_rect.x + 96, pos_rect.y + 73, 16, 17},
  };
  std::vector<bool> r = {
      str_point <= 4, str_point >= 12, dex_point <= 4, dex_point >= 12,
      int_point <= 4, int_point >= 12, luk_point <= 4, luk_point >= 12,
  };

  for (size_t i = 0; i < buttons_nodes.size(); ++i) {
    auto k = buttons_nodes[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    auto &mouse_pos = window::mouse_pos;
    if (r[i]) {
      auto normal = wz_resource::load_texture(k->find(u"disabled/0"));
      SDL_RenderTexture(window::renderer, normal, nullptr, &pos_rect);
    } else if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
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

void character_create_ui_system::render_character() {
  auto &camera = camera_game_instance::camera;
  g_character.pos.x = -84;
  g_character.pos.y = -1211;
  // animate
  character_logic_system::run_animate(g_character);
  character_render_system::render_character(g_character);
}

void character_create_ui_system::render_banner() {
  static auto t = wz_resource::load_texture(
      wz_resource::ui->find(u"Login.img/LoginStart/StepBanner/NewChar"));
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

bool character_create_ui_system::render() {
  render_stat();
  render_button();
  render_character();
  render_backgrnd();
  render_banner();
  return true;
}

bool character_create_ui_system::back_animate() {
  if (login_ui_system::camera_animate(-80, -479)) {
    character_choose_system_instance::enter();
    return false;
  }
  return true;
}

void character_create_ui_system::event_button_back() {
  system::logic_systems.push_back(back_animate);
  system::render_systems = {
      login_system_instance::render_game,
      character_choose_ui_system::render,
      cursor_render_system::render,
  };
  system::event_systems = {};
}

bool character_create_ui_system::event_button(SDL_Event *event) {
  std::vector<SDL_FRect> r;
  std::vector<void (*)()> fns;
  auto &camera = camera_game_instance::camera;
  auto pos = load_pos();
  SDL_FPoint stat_point{
      70 - camera.x,
      -1322 - camera.y,
  };
  r = {
      SDL_FRect{stat_point.x + 48, stat_point.y + 4, 16, 17},
      SDL_FRect{stat_point.x + 96, stat_point.y + 4, 16, 17},
      SDL_FRect{stat_point.x + 48, stat_point.y + 27, 16, 17},
      SDL_FRect{stat_point.x + 96, stat_point.y + 27, 16, 17},
      SDL_FRect{stat_point.x + 48, stat_point.y + 50, 16, 17},
      SDL_FRect{stat_point.x + 96, stat_point.y + 50, 16, 17},
      SDL_FRect{stat_point.x + 48, stat_point.y + 73, 16, 17},
      SDL_FRect{stat_point.x + 96, stat_point.y + 73, 16, 17},
      SDL_FRect{163 + pos.x, 574 + pos.y, 161, 69},
  };
  fns = {
      {}, {}, {}, {}, {}, {}, {}, {}, event_button_back,
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

bool character_create_ui_system::event(SDL_Event *event) {
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