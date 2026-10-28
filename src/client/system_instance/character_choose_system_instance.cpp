#include "character_choose_system_instance.h"
#include "game_save_system_instance.h"
#include "login_system_instance.h"
#include "src/client/game/game_animate.h"
#include "src/client/game_instance/backgrnd_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/obj_game_instance.h"
#include "src/client/game_instance/tile_game_instance.h"
#include "src/client/system/logic/audio_logic_system.h"
#include "src/client/system/logic/backgrnd_logic_system.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/logic/cursor_logic_system.h"
#include "src/client/system/logic/obj_logic_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/character_choose_ui_system.h"
#include "src/common/wz/wz_resource.h"
#include <algorithm>
#include <cstdint>
#include <optional>
#include <ranges>

void CharacterChooseSystemInstance::enter_prepare(const std::string &login) {
  GameSaveSystemInstance::load_save(login);
  // 进一步解析game_save
  auto character_size = GameSaveSystemInstance::save.characters.size();
  CharacterChooseUiSystem::characters.resize(character_size);
  for (int i = 0; i < character_size; i++) {
    auto &sc = GameSaveSystemInstance::save.characters[i];
    CharacterChooseUiSystem::characters[i] = sc.character;
    CharacterLogicSystem::run_stand_action(
        CharacterChooseUiSystem::characters[i]);
    CharacterChooseUiSystem::characters[i].flip = 1;
  }
}

void CharacterChooseSystemInstance::enter(const std::string &login) {
  enter_prepare(login);
  enter();
}

void CharacterChooseSystemInstance::enter() {
  auto fn = &LoginSystemInstance::render_game;
  if (!std::ranges::contains(System::render_systems, fn)) {
    static auto image = WzResource::ui->find(u"MapLogin.img");
    BackgrndGameInstance::load(image);
    ObjGameInstance::load(image);
    TileGameInstance::load(image);
  }
  System::logic_systems = {
      BackgrndLogicSystem::run,      ObjLogicSystem::run,
      AudioLogicSystem::run,         CursorLogicSystem::run,
      CharacterChooseUiSystem::run,
  };
  System::render_systems = {
      LoginSystemInstance::render_game,
      CharacterChooseUiSystem::render,
      CursorRenderSystem::render,
  };
  System::event_systems = {
      CharacterChooseUiSystem::event,
  };
  auto &camera = CameraGameInstance::camera;
  camera.x = -80 - camera.w / 2;
  camera.y = -479 - camera.h / 2;

  CharacterChooseUiSystem::choose = std::nullopt;
  CharacterChooseUiSystem::board.ani_delay = {250, 50, 50, 0};
  CharacterChooseUiSystem::effect.ani_delay = {150, 100, 100, 100, 0};
  auto character_size = CharacterChooseUiSystem::characters.size();
}