#include "src/client/system/ui/login_ui_system.h"

#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/trap_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/button.h"
#include "src/client/system/ui/character_choose_ui_system.h"
#include "src/client/system/ui/login_notice_ui_system.h"
#include "src/client/system/ui/text_input_ui_system.h"
#include "src/client/system_instance/character_choose_system_instance.h"
#include "src/client/system_instance/login_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_main.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <wz/Property.h>

#include <assert.h>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <numeric>
#include <stdio.h>
#include <string>
#include <vector>

namespace sdlms {

#ifdef _WIN32
#define PIPENAME "\\\\.\\pipe\\"
#else
#define PIPENAME ""
#endif

SDL_FPoint LoginUiSystem::LoadPos() {
  SDL_FPoint pos;
  const auto w = 1366;
  const auto h = 768;
  auto& camera = CameraGameInstance::camera_;
  pos.x = (camera.w - w) / 2;
  pos.y = (camera.h - h) / 2;
  return pos;
}

void LoginUiSystem::RenderButton() {
  const static std::array buttons_nodes = {
      WzResource::ui_->find(u"Login.img/ClassicIntro/button:login"),
      WzResource::ui_->find(u"Login.img/ClassicIntro/button:login_saved"),
      WzResource::ui_->find(u"Login.img/ClassicIntro/button:find_id"),
      WzResource::ui_->find(u"Login.img/ClassicIntro/button:find_pw"),
      WzResource::ui_->find(u"Login.img/ClassicIntro/button:register"),
      WzResource::ui_->find(u"Login.img/ClassicIntro/button:homepage"),
      WzResource::ui_->find(u"Login.img/ClassicIntro/button:quit"),
  };
  auto& camera = CameraGameInstance::camera_;
  auto pos = LoadPos();
  std::array buttons_rect = {
      SDL_FRect{160 - camera.x, 960 - camera.y, 122, 61},   // login
      SDL_FRect{-30 - camera.x, 1051 - camera.y, 101, 29},  // login_saved
      SDL_FRect{104 - camera.x, 1047 - camera.y, 90, 29},   // find_id
      SDL_FRect{197 - camera.x, 1047 - camera.y, 74, 29},   // find_pw
      SDL_FRect{-89 - camera.x, 1112 - camera.y, 118, 56},  // register
      SDL_FRect{38 - camera.x, 1112 - camera.y, 119, 55},   // homepage
      SDL_FRect{165 - camera.x, 1112 - camera.y, 108, 55},  // quit
  };
  for (size_t i = 0; i < buttons_nodes.size(); ++i) {
    SDL_FRect rect = buttons_rect[i];
    rect.x = (int)rect.x;
    rect.y = (int)rect.y;
    Button{
        .node = buttons_nodes[i], .rect = rect, .enable = true, .focused = true}
        .Render();
  }
}

void LoginUiSystem::RenderBanner() {
  static auto t = WzResource::LoadTexture(
      WzResource::ui_->find(u"Login.img/LoginStart/StepBanner/ClassicIntro"));
  auto pos = LoadPos();
  SDL_FRect pos_rect{
      pos.x + 160,
      pos.y + 85,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);

  const static std::array buttons_nodes = {
      WzResource::ui_->find(u"Login.img/LoginStart/BtClassicPrev"),
  };
  std::array buttons_rect = {
      SDL_FRect{163 + pos.x, 574 + pos.y, 161, 69},
  };
  for (size_t i = 0; i < buttons_nodes.size(); ++i) {
    auto k = buttons_nodes[i];
    auto pos_rect = buttons_rect[i];
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    auto& mouse_pos = Window::mouse_pos_;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      if (Window::mouse_state_ & SDL_BUTTON_LMASK) {
        auto pressed = WzResource::LoadTexture(k->find(u"pressed/0"));
        SDL_RenderTexture(Window::renderer_, pressed, nullptr, &pos_rect);
      } else {
        auto mouse_over = WzResource::LoadTexture(k->find(u"mouseOver/0"));
        SDL_RenderTexture(Window::renderer_, mouse_over, nullptr, &pos_rect);
      }
    } else {
      auto normal = WzResource::LoadTexture(k->find(u"normal/0"));
      SDL_RenderTexture(Window::renderer_, normal, nullptr, &pos_rect);
    }
  }
}

void LoginUiSystem::RenderBackgrnd() {
  static auto t = WzResource::LoadTexture(
      WzResource::ui_->find(u"Login.img/Common/classicFrame"));
  auto pos = LoadPos();
  SDL_FRect pos_rect{
      pos.x,
      pos.y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
}

void LoginUiSystem::RenderEffect() {
  auto pos = LoadPos();
  const static std::array effect_nodes = {
      WzResource::ui_->find(u"Login.img/ClassicIntro/animation:effect0"),
      WzResource::ui_->find(u"Login.img/ClassicIntro/animation:effect1"),
      WzResource::ui_->find(u"Login.img/ClassicIntro/animation:effect2"),
      WzResource::ui_->find(u"Login.img/ClassicIntro/animation:effect3"),
      WzResource::ui_->find(u"Login.img/ClassicIntro/animation:effect4"),
      WzResource::ui_->find(u"Login.img/ClassicIntro/animation:effect5"),
  };
  const static std::vector<std::vector<int>> delays = {
      {1500, 1500, 1000},  //
      {2500, 2500},        //
      {1700, 1700, 1700},  //
      {3000, 3000},        //
      {2800, 2800},        //
      {1000, 1000, 2500},  //
  };
  auto& camera = CameraGameInstance::camera_;
  const std::vector<SDL_FPoint> origins = {
      {75 - camera.x, 502 - camera.y},   {-121 - camera.x, 438 - camera.y},
      {-145 - camera.x, 448 - camera.y}, {39 - camera.x, 478 - camera.y},
      {-155 - camera.x, 451 - camera.y}, {-153 - camera.x, 435 - camera.y},
  };
  for (auto i = 0; i < effect_nodes.size(); i++) {
    auto ds = delays[i];
    // render
    auto AnimateData = TrapGameInstance::LoadAnimateIndex(ds);
    auto render_index = AnimateData.index;
    auto index = std::to_string(render_index);
    auto node = effect_nodes[i]->get_child(index);
    uint8_t a0 = 255;
    if (node->get_child(u"a0")) {
      a0 = static_cast<wz::Property<int>*>(node->get_child(u"a0"))->get();
    }
    uint8_t a1 = 255;
    if (node->get_child(u"a1")) {
      a1 = static_cast<wz::Property<int>*>(node->get_child(u"a1"))->get();
    }
    auto ani_time = AnimateData.time;
    float t = (float)ani_time / (float)ds[render_index];
    auto alpha = a0 + (a1 - a0) * t;
    auto texture = WzResource::LoadTexture(node);
    SDL_FRect pos_rect{
        origins[i].x,
        origins[i].y,
        static_cast<float>(texture->w),
        static_cast<float>(texture->h),
    };
    SDL_SetTextureAlphaMod(texture, alpha);
    SDL_RenderTexture(Window::renderer_, texture, nullptr, &pos_rect);
  }
}

void LoginUiSystem::RenderUsername() {
  TextInputUiSystem::Render(username_, 5, 5);
}

bool LoginUiSystem::Render() {
  RenderButton();
  RenderEffect();
  RenderBackgrnd();
  RenderBanner();
  RenderUsername();
  return true;
}

bool LoginUiSystem::CameraAnimate(float x, float y) {
  auto& camera = CameraGameInstance::camera_;
  auto prev_x = camera.x;
  auto next_x = x - camera.w / 2;  // 人物移动后新的摄像机坐标
  auto delta_x = next_x - prev_x;

  camera.x = (std::lerp(prev_x, next_x, std::abs(delta_x) / 6000.0f));

  auto prev_y = camera.y;
  auto next_y = y - camera.h / 2;  // 人物移动后新的摄像机坐标
  auto delta_y = next_y - prev_y;

  const float MIN_T = 0.4f;  // 最小lerp系数
  float t = std::abs(delta_y) / 6000.0f;
  t = std::max(t, MIN_T);

  camera.y = std::lerp(prev_y, next_y, t);

  if (std::roundf(camera.x) == next_x && std::roundf(camera.y) == next_y) {
    return true;
  } else {
    return false;
  }
}

bool LoginUiSystem::LoginAnimate() {
  TextInputUiSystem::Close(username_);
  if (CameraAnimate(-80, -479)) {
    CharacterChooseSystemInstance::Enter();
    return false;
  }
  return true;
}

void LoginUiSystem::EventButtonLogin() {
  if (username_.text.empty()) {
    return;
  }
  static std::string name;
  int init_result;
  name = {username_.text.begin(), username_.text.end()};
  // 命名管道检测是否多次打开
  name = "__sdlMS__" + name;
  name = PIPENAME + name;

  init_result = uv_pipe_init(ServerMain::loop_, &g_pipe_, 0);
  if (init_result != 0) {
    assert(0);
    std::abort();
  }
  static bool con_connect;
  con_connect = false;
  static uv_connect_t c;
  uv_pipe_connect(&c, &g_pipe_, name.c_str(),
                  [](uv_connect_t* req, int status) {
                    if (status != 0) {
                      uv_fs_t req;
                      uv_fs_unlink(NULL, &req, name.c_str(), NULL);
                      uv_fs_req_cleanup(&req);
                    }
                    con_connect = true;
                    uv_close((uv_handle_t*)&g_pipe_, NULL);
                  });
  // 阻塞等待
  while (!con_connect) {
    uv_run(ServerMain::loop_, UV_RUN_ONCE);
  }
  init_result = uv_pipe_init(ServerMain::loop_, &g_pipe_, 0);
  if (init_result != 0) {
    assert(0);
    std::abort();
  }
  auto bind_result = uv_pipe_bind(&g_pipe_, name.c_str());
  if (bind_result != 0) {
    // 绑定失败
    LoginNoticeSystemInstance::Enter(
        LoginNoticeSystemInstance::NoticeType::kLoginingUsed, nullptr);
    AudioGameInstance::LoadAudio(u"UI.img/DlgNotice", 0);
    return;
  }

  CharacterChooseSystemInstance::EnterPrepare(name);
  System::logic_systems_.push_back(LoginAnimate);
  System::render_systems_ = {
      LoginSystemInstance::RenderGame,
      CharacterChooseUiSystem::Render,
      CursorRenderSystem::Render,
  };
  System::event_systems_ = {};
  AudioGameInstance::LoadAudio(u"UI.img/ScrollUp", 0);
}

void LoginUiSystem::EventButtonLoginSave() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
}

void LoginUiSystem::EventButtonFindId() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
}

void LoginUiSystem::EventButtonFindPw() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
}

void LoginUiSystem::EventButtonRegister() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
}
void LoginUiSystem::EventButtonHomepage() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
}

void LoginUiSystem::EventButtonQuit() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
}

void LoginUiSystem::EventButtonBack() {
  EventButtonQuit();
}

bool LoginUiSystem::EventButton(SDL_Event* event) {
  std::vector<SDL_FRect> r;
  std::vector<void (*)()> fns;
  auto& camera = CameraGameInstance::camera_;
  auto pos = LoadPos();
  r = {
      SDL_FRect{160 - camera.x, 960 - camera.y, 122, 61},   // login
      SDL_FRect{-30 - camera.x, 1051 - camera.y, 101, 29},  // login_saved
      SDL_FRect{104 - camera.x, 1047 - camera.y, 90, 29},   // find_id
      SDL_FRect{197 - camera.x, 1047 - camera.y, 74, 29},   // find_pw
      SDL_FRect{-89 - camera.x, 1112 - camera.y, 118, 56},  // register
      SDL_FRect{38 - camera.x, 1112 - camera.y, 119, 55},   // homepage
      SDL_FRect{165 - camera.x, 1112 - camera.y, 108, 55},  // quit
      SDL_FRect{163 + pos.x, 574 + pos.y, 161, 69},         // BtClassicPrev
  };
  fns = {EventButtonLogin,  EventButtonLoginSave, EventButtonFindId,
         EventButtonFindPw, EventButtonRegister,  EventButtonHomepage,
         EventButtonQuit,   EventButtonBack};
  for (size_t i = 0; i < r.size(); ++i) {
    auto pos_rect = r[i];
    pos_rect.x = (int)pos_rect.x;
    pos_rect.y = (int)pos_rect.y;
    if (SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect)) {
      fns[i]();
      return false;
    }
  }

  return false;
}

bool LoginUiSystem::Event(SDL_Event* event) {
  TextInputUiSystem::Event(event, username_);
  switch (event->type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (event->button.button == SDL_BUTTON_LEFT) {
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        EventButton(event);
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

}  // namespace sdlms
