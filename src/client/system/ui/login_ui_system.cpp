#include "login_ui_system.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "login_notice_ui_system.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/trap_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/character_choose_ui_system.h"
#include "src/client/system_instance/character_choose_system_instance.h"
#include "src/client/system_instance/login_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_main.h"
#include "text_input_ui_system.h"
#include "wz/Property.h"
#include <assert.h>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <numeric>
#include <stdio.h>
#include <string>
#include <vector>

#ifdef _WIN32
#define PIPENAME "\\\\.\\pipe\\"
#else
#define PIPENAME ""
#endif

SDL_FPoint LoginUiSystem::load_pos() {
  SDL_FPoint pos;
  const auto w = 1366;
  const auto h = 768;
  auto &camera = CameraGameInstance::camera;
  pos.x = (camera.w - w) / 2;
  pos.y = (camera.h - h) / 2;
  return pos;
}

void LoginUiSystem::render_button() {
  const static std::array buttons_nodes = {
      WzResource::ui->find(u"Login.img/ClassicIntro/button:login"),
      WzResource::ui->find(u"Login.img/ClassicIntro/button:login_saved"),
      WzResource::ui->find(u"Login.img/ClassicIntro/button:find_id"),
      WzResource::ui->find(u"Login.img/ClassicIntro/button:find_pw"),
      WzResource::ui->find(u"Login.img/ClassicIntro/button:register"),
      WzResource::ui->find(u"Login.img/ClassicIntro/button:homepage"),
      WzResource::ui->find(u"Login.img/ClassicIntro/button:quit"),
  };
  auto &camera = CameraGameInstance::camera;
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
    auto &mouse_pos = Window::mouse_pos;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      if (Window::mouse_state & SDL_BUTTON_LMASK) {
        auto pressed = WzResource::load_texture(k->find(u"pressed/0"));
        SDL_RenderTexture(Window::renderer, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = WzResource::load_texture(k->find(u"mouseOver/0"));
        SDL_RenderTexture(Window::renderer, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = WzResource::load_texture(k->find(u"normal/0"));
      SDL_RenderTexture(Window::renderer, normal, nullptr, &pos_rect);
    }
  }
}

void LoginUiSystem::render_banner() {
  static auto t = WzResource::load_texture(
      WzResource::ui->find(u"Login.img/LoginStart/StepBanner/ClassicIntro"));
  auto pos = load_pos();
  SDL_FRect pos_rect{
      pos.x + 160,
      pos.y + 85,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);

  const static std::array buttons_nodes = {
      WzResource::ui->find(u"Login.img/LoginStart/BtClassicPrev"),
  };
  std::array buttons_rect = {
      SDL_FRect{163 + pos.x, 574 + pos.y, 161, 69},
  };
  for (size_t i = 0; i < buttons_nodes.size(); ++i) {
    auto k = buttons_nodes[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    auto &mouse_pos = Window::mouse_pos;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      if (Window::mouse_state & SDL_BUTTON_LMASK) {
        auto pressed = WzResource::load_texture(k->find(u"pressed/0"));
        SDL_RenderTexture(Window::renderer, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = WzResource::load_texture(k->find(u"mouseOver/0"));
        SDL_RenderTexture(Window::renderer, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = WzResource::load_texture(k->find(u"normal/0"));
      SDL_RenderTexture(Window::renderer, normal, nullptr, &pos_rect);
    }
  }
}

void LoginUiSystem::render_backgrnd() {
  static auto t = WzResource::load_texture(
      WzResource::ui->find(u"Login.img/Common/classicFrame"));
  auto pos = load_pos();
  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer, t, nullptr, &pos_rect);
}

void LoginUiSystem::render_effect() {
  auto pos = load_pos();
  const static std::array effect_nodes = {
      WzResource::ui->find(u"Login.img/ClassicIntro/animation:effect0"),
      WzResource::ui->find(u"Login.img/ClassicIntro/animation:effect1"),
      WzResource::ui->find(u"Login.img/ClassicIntro/animation:effect2"),
      WzResource::ui->find(u"Login.img/ClassicIntro/animation:effect3"),
      WzResource::ui->find(u"Login.img/ClassicIntro/animation:effect4"),
      WzResource::ui->find(u"Login.img/ClassicIntro/animation:effect5"),
  };
  const static std::vector<std::vector<int>> delays = {
      {1500, 1500, 1000}, //
      {2500, 2500},       //
      {1700, 1700, 1700}, //
      {3000, 3000},       //
      {2800, 2800},       //
      {1000, 1000, 2500}, //
  };
  auto &camera = CameraGameInstance::camera;
  const std::vector<SDL_FPoint> origins = {
      {75 - camera.x, 502 - camera.y},   {-121 - camera.x, 438 - camera.y},
      {-145 - camera.x, 448 - camera.y}, {39 - camera.x, 478 - camera.y},
      {-155 - camera.x, 451 - camera.y}, {-153 - camera.x, 435 - camera.y},
  };
  for (auto i = 0; i < effect_nodes.size(); i++) {
    auto ds = delays[i];
    // render
    auto AnimateData = TrapGameInstance::load_animate_index(ds);
    auto render_index = AnimateData.index;
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
    auto ani_time = AnimateData.time;
    float t = (float)ani_time / (float)ds[render_index];
    auto alpha = a0 + (a1 - a0) * t;
    auto texture = WzResource::load_texture(node);
    SDL_FRect pos_rect{
        origins[i].x,
        origins[i].y,
        static_cast<float>(texture->w),
        static_cast<float>(texture->h),
    };
    SDL_SetTextureAlphaMod(texture, alpha);
    SDL_RenderTexture(Window::renderer, texture, nullptr, &pos_rect);
  }
}

void LoginUiSystem::render_username() {
  TextInputUiSystem::render(username, 5, 5);
}

bool LoginUiSystem::render() {
  render_button();
  render_effect();
  render_backgrnd();
  render_banner();
  render_username();
  return true;
}

bool LoginUiSystem::camera_animate(float x, float y) {
  auto &camera = CameraGameInstance::camera;
  auto prev_x = camera.x;
  auto next_x = x - camera.w / 2; // 人物移动后新的摄像机坐标
  auto delta_x = next_x - prev_x;

  camera.x = (std::lerp(prev_x, next_x, std::abs(delta_x) / 6000.0f));

  auto prev_y = camera.y;
  auto next_y = y - camera.h / 2; // 人物移动后新的摄像机坐标
  auto delta_y = next_y - prev_y;

  const float MIN_T = 0.4f; // 最小lerp系数
  float t = std::abs(delta_y) / 6000.0f;
  t = std::max(t, MIN_T);

  camera.y = std::lerp(prev_y, next_y, t);

  if (std::roundf(camera.x) == next_x && std::roundf(camera.y) == next_y) {
    return true;
  } else {
    return false;
  }
}

bool LoginUiSystem::login_animate() {
  TextInputUiSystem::close(username);
  if (camera_animate(-80, -479)) {
    CharacterChooseSystemInstance::enter();
    return false;
  }
  return true;
}

void LoginUiSystem::event_button_login() {
  if (username.text.empty()) {
    return;
  }
  static std::string name;
  int init_result;
  name = {username.text.begin(), username.text.end()};
  // 命名管道检测是否多次打开
  name = "__sdlMS__" + name;
  name = PIPENAME + name;

  init_result = uv_pipe_init(ServerMain::loop, &g_pipe, 0);
  if (init_result != 0) {
    assert(0);
    std::abort();
  }
  static bool con_connect;
  con_connect = false;
  static uv_connect_t c;
  uv_pipe_connect(&c, &g_pipe, name.c_str(), [](uv_connect_t *req, int status) {
    if (status != 0) {
      uv_fs_t req;
      uv_fs_unlink(NULL, &req, name.c_str(), NULL);
      uv_fs_req_cleanup(&req);
    }
    con_connect = true;
    uv_close((uv_handle_t *)&g_pipe, NULL);
  });
  // 阻塞等待
  while (!con_connect) {
    uv_run(ServerMain::loop, UV_RUN_ONCE);
  }
  init_result = uv_pipe_init(ServerMain::loop, &g_pipe, 0);
  if (init_result != 0) {
    assert(0);
    std::abort();
  }
  auto bind_result = uv_pipe_bind(&g_pipe, name.c_str());
  if (bind_result != 0) {
    // 绑定失败
    LoginNoticeSystemInstance::enter(
        LoginNoticeSystemInstance::NoticeType::kLoginingUsed, nullptr);
    AudioGameInstance::load_audio(u"UI.img/DlgNotice", 0);
    return;
  }

  CharacterChooseSystemInstance::enter_prepare(name);
  System::logic_systems.push_back(login_animate);
  System::render_systems = {
      LoginSystemInstance::render_game,
      CharacterChooseUiSystem::render,
      CursorRenderSystem::render,
  };
  System::event_systems = {};
  AudioGameInstance::load_audio(u"UI.img/ScrollUp", 0);
}

void LoginUiSystem::event_button_login_save() {
  AudioGameInstance::load_audio(u"UI.img/BtMouseClick", 0);
}

void LoginUiSystem::event_button_find_id() {
  AudioGameInstance::load_audio(u"UI.img/BtMouseClick", 0);
}

void LoginUiSystem::event_button_find_pw() {
  AudioGameInstance::load_audio(u"UI.img/BtMouseClick", 0);
}

void LoginUiSystem::event_button_register() {
  AudioGameInstance::load_audio(u"UI.img/BtMouseClick", 0);
}
void LoginUiSystem::event_button_homepage() {
  AudioGameInstance::load_audio(u"UI.img/BtMouseClick", 0);
}

void LoginUiSystem::event_button_quit() {
  AudioGameInstance::load_audio(u"UI.img/BtMouseClick", 0);
}

void LoginUiSystem::event_button_back() { event_button_quit(); }

bool LoginUiSystem::event_button(SDL_Event *event) {
  std::vector<SDL_FRect> r;
  std::vector<void (*)()> fns;
  auto &camera = CameraGameInstance::camera;
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
    if (SDL_PointInRectFloat(&Window::mouse_pos, &pos_rect)) {
      fns[i]();
      return false;
    }
  }

  return false;
}

bool LoginUiSystem::event(SDL_Event *event) {
  TextInputUiSystem::event(event, username);
  switch (event->type) {
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