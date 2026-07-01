#include "login_system_instance.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/backgrnd_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/obj_game_instance.h"
#include "src/client/game_instance/tile_game_instance.h"
#include "src/client/system/logic/audio_logic_system.h"
#include "src/client/system/logic/backgrnd_logic_system.h"
#include "src/client/system/logic/cursor_logic_system.h"
#include "src/client/system/logic/obj_logic_system.h"
#include "src/client/system/render/backgrnd_render_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/render/obj_render_system.h"
#include "src/client/system/render/tile_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/login_ui_system.h"
#include "src/common/wz/wz_resource.h"
#include <algorithm>
#include <ranges>

bool login_system_instance::render_game() {
  //   后景
  for (auto &b_backgrnd : backgrnd_game_instance::back | std::views::values) {
    backgrnd_render_system::render(b_backgrnd);
  }
  for (uint8_t i = 0; i < 8; i++) {
    for (auto &obj : obj_game_instance::data[i] | std::views::values) {
      obj_render_system::render(obj);
    }
    for (auto &tile : tile_game_instance::data[i] | std::views::values) {
      tile_render_system::render(tile);
    }
  }
  // 前景
  for (auto &f_backgrnd : backgrnd_game_instance::front | std::views::values) {
    backgrnd_render_system::render(f_backgrnd);
  }
  return true;
}

void login_system_instance::enter_prepare() {
  auto camera = camera_game_instance::camera;
  camera.x = -80 - camera.w / 2;
  camera.y = 1023 - camera.h / 2;
  login_ui_system::username = {
      .max_size = 12,
      .text = u"sdlMS",
      .composition = {},
      .disable = false,
      .active = false,
      .r =
          SDL_Rect{
              static_cast<int>(-30 - camera.x),
              static_cast<int>(960 - camera.y),
              230,
              30,
          },
      .font_color = {255, 255, 255, 255},
      .font_size = 13,
  };
  login_ui_system::username.type.set(text_input::letter);
  login_ui_system::username.type.set(text_input::digit);
  if (login_ui_system::g_pipe.type == UV_NAMED_PIPE) {
    uv_close((uv_handle_t *)&login_ui_system::g_pipe, nullptr);
    login_ui_system::g_pipe = {};
  }
}

void login_system_instance::enter() {
  audio_game_instance::load_backgrnd_audio(u"BgmUI.img/TitleClassic");

  enter_prepare();
  auto fn = &login_system_instance::render_game;
  if (!std::ranges::contains(system::render_systems, fn)) {
    static auto image = wz_resource::ui->find(u"MapLogin.img");
    backgrnd_game_instance::load(image);
    obj_game_instance::load(image);
    tile_game_instance::load(image);
  }

  system::logic_systems = {
      backgrnd_logic_system::run,
      obj_logic_system::run,
      audio_logic_system::run,
      cursor_logic_system::run,
  };
  system::render_systems = {
      render_game,
      login_ui_system::render,
      cursor_render_system::render,
  };
  system::event_systems = {
      login_ui_system::event,
  };
  auto &camera = camera_game_instance::camera;
  camera.x = -80 - camera.w / 2;
  camera.y = 1023 - camera.h / 2;
}