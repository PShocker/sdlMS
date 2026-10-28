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

void CharacterChooseSystemInstance::EnterPrepare(const std::string &login) {
  GameSaveSystemInstance::LoadSave(login);
  // 进一步解析game_save
  auto character_size = GameSaveSystemInstance::save_.characters.size();
  CharacterChooseUiSystem::characters_.resize(character_size);
  for (int i = 0; i < character_size; i++) {
    auto &sc = GameSaveSystemInstance::save_.characters[i];
    CharacterChooseUiSystem::characters_[i] = sc.character;
    CharacterLogicSystem::RunStandAction(
        CharacterChooseUiSystem::characters_[i]);
    CharacterChooseUiSystem::characters_[i].flip = 1;
  }
}

void CharacterChooseSystemInstance::Enter(const std::string &login) {
  EnterPrepare(login);
  Enter();
}

void CharacterChooseSystemInstance::Enter() {
  auto fn = &LoginSystemInstance::RenderGame;
  if (!std::ranges::contains(System::render_systems_, fn)) {
    static auto image = WzResource::ui_->find(u"MapLogin.img");
    BackgrndGameInstance::Load(image);
    ObjGameInstance::Load(image);
    TileGameInstance::Load(image);
  }
  System::logic_systems_ = {
      BackgrndLogicSystem::Run,      ObjLogicSystem::Run,
      AudioLogicSystem::Run,         CursorLogicSystem::Run,
      CharacterChooseUiSystem::Run,
  };
  System::render_systems_ = {
      LoginSystemInstance::RenderGame,
      CharacterChooseUiSystem::Render,
      CursorRenderSystem::Render,
  };
  System::event_systems_ = {
      CharacterChooseUiSystem::Event,
  };
  auto &camera = CameraGameInstance::camera_;
  camera.x = -80 - camera.w / 2;
  camera.y = -479 - camera.h / 2;

  CharacterChooseUiSystem::choose_ = std::nullopt;
  CharacterChooseUiSystem::board_.ani_delay = {250, 50, 50, 0};
  CharacterChooseUiSystem::effect_.ani_delay = {150, 100, 100, 100, 0};
  auto character_size = CharacterChooseUiSystem::characters_.size();
}