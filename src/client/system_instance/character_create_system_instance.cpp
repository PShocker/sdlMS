#include "character_create_system_instance.h"
#include "login_system_instance.h"
#include "src/client/game_instance/backgrnd_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/obj_game_instance.h"
#include "src/client/game_instance/tile_game_instance.h"
#include "src/client/system/logic/backgrnd_logic_system.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/logic/cursor_logic_system.h"
#include "src/client/system/logic/obj_logic_system.h"
#include "src/client/system/logic/audio_logic_system.h"
#include "src/client/system/render/backgrnd_render_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/render/obj_render_system.h"
#include "src/client/system/render/tile_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/character_create_ui_system.h"
#include "src/common/wz/wz_resource.h"
#include <algorithm>
#include <ranges>

void CharacterCreateSystemInstance::EnterPrepare() {
  CharacterCreateUiSystem::ResetCharacter(false);
  CharacterCreateUiSystem::str_point = 4;
  CharacterCreateUiSystem::dex_point = 4;
  CharacterCreateUiSystem::int_point = 4;
  CharacterCreateUiSystem::luk_point = 4;
  CharacterCreateUiSystem::remain_point = 9;
}

void CharacterCreateSystemInstance::Enter() {
  EnterPrepare();
  auto &camera = CameraGameInstance::camera;
  camera.x = -80 - camera.w / 2;
  camera.y = -1294 - camera.h / 2;

  CharacterCreateUiSystem::name = {
      .max_size = 10,
      .text = {},
      .composition = {},
      .disable = false,
      .active = false,
      .r =
          SDL_Rect{
              static_cast<int>(65 - camera.x),
              static_cast<int>(-1400 - camera.y),
              235,
              30,
          },
      .font_color = {255, 255, 255, 255},
      .font_size = 13,
  };
  CharacterCreateUiSystem::name.type.set(TextInput::kIme);

  auto fn = &LoginSystemInstance::RenderGame;
  if (!std::ranges::contains(System::render_systems, fn)) {
    static auto image = WzResource::ui->find(u"MapLogin.img");
    BackgrndGameInstance::Load(image);
    ObjGameInstance::Load(image);
    TileGameInstance::Load(image);
  }

  System::logic_systems = {
      BackgrndLogicSystem::Run,      ObjLogicSystem::Run,
      AudioLogicSystem::Run,         CursorLogicSystem::Run,
      CharacterCreateUiSystem::Run,
  };
  System::render_systems = {
      LoginSystemInstance::RenderGame,
      CharacterCreateUiSystem::Render,
      CursorRenderSystem::Render,
  };
  System::event_systems = {
      CharacterCreateUiSystem::Event,
  };
}