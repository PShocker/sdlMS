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

bool LoginSystemInstance::RenderGame() {
  //   后景
  for (auto &b_backgrnd : BackgrndGameInstance::back | std::views::values) {
    BackgrndRenderSystem::Render(b_backgrnd);
  }
  for (uint8_t i = 0; i < 8; i++) {
    for (auto &obj : ObjGameInstance::data[i] | std::views::values) {
      ObjRenderSystem::Render(obj);
    }
    for (auto &tile : TileGameInstance::data[i] | std::views::values) {
      TileRenderSystem::Render(tile);
    }
  }
  // 前景
  for (auto &f_backgrnd : BackgrndGameInstance::front | std::views::values) {
    BackgrndRenderSystem::Render(f_backgrnd);
  }
  return true;
}

void LoginSystemInstance::EnterPrepare() {
  auto camera = CameraGameInstance::camera;
  camera.x = -80 - camera.w / 2;
  camera.y = 1023 - camera.h / 2;
  LoginUiSystem::username = {
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
  LoginUiSystem::username.type.set(TextInput::kLetter);
  LoginUiSystem::username.type.set(TextInput::kDigit);
  if (LoginUiSystem::g_pipe.type == UV_NAMED_PIPE) {
    uv_close((uv_handle_t *)&LoginUiSystem::g_pipe, nullptr);
    LoginUiSystem::g_pipe = {};
  }
}

void LoginSystemInstance::Enter() {
  AudioGameInstance::LoadBackgrndAudio(u"BgmUI.img/TitleClassic");

  EnterPrepare();
  auto fn = &LoginSystemInstance::RenderGame;
  if (!std::ranges::contains(System::render_systems, fn)) {
    static auto image = WzResource::ui->find(u"MapLogin.img");
    BackgrndGameInstance::Load(image);
    ObjGameInstance::Load(image);
    TileGameInstance::Load(image);
  }

  System::logic_systems = {
      BackgrndLogicSystem::Run,
      ObjLogicSystem::Run,
      AudioLogicSystem::Run,
      CursorLogicSystem::Run,
  };
  System::render_systems = {
      RenderGame,
      LoginUiSystem::Render,
      CursorRenderSystem::Render,
  };
  System::event_systems = {
      LoginUiSystem::Event,
  };
  auto &camera = CameraGameInstance::camera;
  camera.x = -80 - camera.w / 2;
  camera.y = 1023 - camera.h / 2;
}