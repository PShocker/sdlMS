#include "login_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/character_choose_ui_system.h"
#include "src/client/system_instance/character_choose_system_instance.h"
#include "src/client/system_instance/login_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <numeric>
#include <string>
#include <vector>

SDL_FPoint login_ui_system::load_pos() {
  SDL_FPoint pos;
  const auto w = 1366;
  const auto h = 768;
  auto &camera = camera_game_instance::camera;
  pos.x = (camera.w - w) / 2;
  pos.y = (camera.h - h) / 2;
  return pos;
}

void login_ui_system::render_button() {
  const static std::array buttons_nodes = {
      wz_resource::ui->find(u"Login.img/ClassicIntro/button:login"),
      wz_resource::ui->find(u"Login.img/ClassicIntro/button:login_saved"),
      wz_resource::ui->find(u"Login.img/ClassicIntro/button:find_id"),
      wz_resource::ui->find(u"Login.img/ClassicIntro/button:find_pw"),
      wz_resource::ui->find(u"Login.img/ClassicIntro/button:register"),
      wz_resource::ui->find(u"Login.img/ClassicIntro/button:homepage"),
      wz_resource::ui->find(u"Login.img/ClassicIntro/button:quit"),
  };
  auto &camera = camera_game_instance::camera;
  auto pos = load_pos();
  std::array buttons_rect = {
      SDL_FRect{160 - camera.x, 960 - camera.y, 122, 61},  // login
      SDL_FRect{-30 - camera.x, 1051 - camera.y, 101, 29}, // login_saved
      SDL_FRect{104 - camera.x, 1047 - camera.y, 90, 29},  // find_id
      SDL_FRect{197 - camera.x, 1047 - camera.y, 74, 29},  // find_pw
      SDL_FRect{-89 - camera.x, 1112 - camera.y, 118, 56}, // register
      SDL_FRect{38 - camera.x, 1112 - camera.y, 119, 55},  // homepage
      SDL_FRect{165 - camera.x, 1112 - camera.y, 108, 55}, // quit
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

void login_ui_system::render_banner() {
  static auto t = wz_resource::load_texture(
      wz_resource::ui->find(u"Login.img/LoginStart/StepBanner/ClassicIntro"));
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

void login_ui_system::render_backgrnd() {
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

void login_ui_system::render_effect() {
  auto pos = load_pos();
  const static std::array effect_nodes = {
      wz_resource::ui->find(u"Login.img/ClassicIntro/animation:effect0"),
      wz_resource::ui->find(u"Login.img/ClassicIntro/animation:effect1"),
      wz_resource::ui->find(u"Login.img/ClassicIntro/animation:effect2"),
      wz_resource::ui->find(u"Login.img/ClassicIntro/animation:effect3"),
      wz_resource::ui->find(u"Login.img/ClassicIntro/animation:effect4"),
      wz_resource::ui->find(u"Login.img/ClassicIntro/animation:effect5"),
  };
  const static std::vector<std::vector<uint32_t>> delays = {
      {1500, 1500, 1000}, //
      {2500, 2500},       //
      {1700, 1700, 1700}, //
      {3000, 3000},       //
      {2800, 2800},       //
      {1000, 1000, 2500}, //
  };
  auto &camera = camera_game_instance::camera;
  const std::vector<SDL_FPoint> origins = {
      {75 - camera.x, 502 - camera.y},   {-121 - camera.x, 438 - camera.y},
      {-145 - camera.x, 448 - camera.y}, {39 - camera.x, 478 - camera.y},
      {-155 - camera.x, 451 - camera.y}, {-153 - camera.x, 435 - camera.y},
  };
  for (auto i = 0; i < effect_nodes.size(); i++) {
    auto ds = delays[i];
    auto sum = std::accumulate(ds.begin(), ds.end(), 0u);
    auto offset = window::dt_now % sum; // 取余，得到周期内偏移
    uint32_t accumulated = 0;
    uint8_t render_index = 0;
    for (auto j = 0; j < ds.size(); j++) {
      if (offset < accumulated + ds[j]) {
        render_index = j;
        break;
      }
      accumulated += ds[j];
    }
    // render
    auto index = std::to_string(render_index);
    auto node = effect_nodes[i]->get_child(index);
    uint8_t a0 = 255;
    if (node->get_child(u"a0")) {
      a0 = static_cast<wz::Property<int> *>(node->get_child(u"a0"))->get();
    }
    uint8_t a1 = 255;
    if (node->get_child(u"a1")) {
      a1 = static_cast<wz::Property<int> *>(node->get_child(u"a1"))->get();
    }
    auto ani_time = offset - accumulated;
    float t = (float)ani_time / (float)ds[render_index];
    auto alpha = a0 + (a1 - a0) * t;
    auto texture = wz_resource::load_texture(node);
    SDL_FRect pos_rect{
        origins[i].x,
        origins[i].y,
        static_cast<float>(texture->w),
        static_cast<float>(texture->h),
    };
    SDL_SetTextureAlphaMod(texture, alpha);
    SDL_RenderTexture(window::renderer, texture, nullptr, &pos_rect);
  }
}

bool login_ui_system::render() {
  render_button();
  render_effect();
  render_backgrnd();
  render_banner();
  return true;
}

bool login_ui_system::login_animate() {
  const auto x = -80;
  const auto y = -479;

  auto &camera = camera_game_instance::camera;
  auto prev_x = camera.x;
  auto next_x = x - camera.w / 2; // 人物移动后新的摄像机坐标
  auto delta_x = next_x - prev_x;

  camera.x = (std::lerp(prev_x, next_x, std::abs(delta_x) / 6000.0f));

  auto prev_y = camera.y;
  auto next_y = y - camera.h / 2; // 人物移动后新的摄像机坐标
  auto delta_y = next_y - prev_y;

  const float MIN_T = 0.2f; // 最小lerp系数
  float t = std::abs(delta_y) / 6000.0f;
  t = std::max(t, MIN_T);

  camera.y = std::lerp(prev_y, next_y, t);

  if (std::roundf(camera.x) == next_x && std::roundf(camera.y) == next_y) {
    character_choose_system_instance::enter();
    return false;
  } else {
    return true;
  }
}

void login_ui_system::event_button_login() {
  system::logic_systems.push_back(login_animate);
  system::render_systems = {
      login_system_instance::render_game,
      character_choose_ui_system::render,
      cursor_render_system::render,
  };
  system::event_systems = {};
}

void login_ui_system::event_button_login_save() {}

void login_ui_system::event_button_find_id() {}

void login_ui_system::event_button_find_pw() {}

void login_ui_system::event_button_register() {}
void login_ui_system::event_button_homepage() {}

void login_ui_system::event_button_quit() {}
void login_ui_system::event_button_back() { event_button_quit(); }
bool login_ui_system::event_button(SDL_Event *event) {
  std::vector<SDL_FRect> r;
  std::vector<void (*)()> fns;
  auto &camera = camera_game_instance::camera;
  auto pos = load_pos();
  r = {
      SDL_FRect{160 - camera.x, 960 - camera.y, 122, 61},  // login
      SDL_FRect{-30 - camera.x, 1051 - camera.y, 101, 29}, // login_saved
      SDL_FRect{104 - camera.x, 1047 - camera.y, 90, 29},  // find_id
      SDL_FRect{197 - camera.x, 1047 - camera.y, 74, 29},  // find_pw
      SDL_FRect{-89 - camera.x, 1112 - camera.y, 118, 56}, // register
      SDL_FRect{38 - camera.x, 1112 - camera.y, 119, 55},  // homepage
      SDL_FRect{165 - camera.x, 1112 - camera.y, 108, 55}, // quit
      SDL_FRect{163 + pos.x, 574 + pos.y, 161, 69},        // BtClassicPrev
  };
  fns = {event_button_login,   event_button_login_save, event_button_find_id,
         event_button_find_pw, event_button_register,   event_button_homepage,
         event_button_quit,    event_button_back};
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

bool login_ui_system::event(SDL_Event *event) {
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
  return true;
}