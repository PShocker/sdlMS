#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/keyboard_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/quest_game_instance.h"
#include "src/client/game_instance/shader_game_instance.h"
#include "src/client/system/system.h"
#include "src/client/system_instance/game_save_system_instance.h"
#include "src/client/system_instance/login_system_instance.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_main.h"
#include "src/server/server_system/server_system.h"

#include <cstdint>
#include <cstdlib>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

namespace sdlms {

namespace {
// Logical resolution.
constexpr int32_t kLogicWidth = 1366;
constexpr int32_t kLogicHeight = 768;

// Window resolution.
constexpr int32_t kWindowWidth = 1366;
constexpr int32_t kWindowHeight = 768;

}  // namespace

SDL_AppResult AppIterate(void* appstate) {
  Window::Tick();
  Window::Clear();
  for (const auto& fn : System::logic_systems_) {
    if (!fn()) {
      break;
    }
  }

  // ShaderGameInstance::Start(ShaderEnum::kGrayscale);
  for (const auto& fn : System::render_systems_) {
    if (!fn()) {
      break;
    }
  }
  // ShaderGameInstance::End();

  Window::Update();
  for (const auto& fn : ServerSystem::server_systems_) {
    if (fn() == false) {
      break;
    }
  }
  return SDL_APP_CONTINUE;
}

SDL_AppResult AppInit(void** appstate, int argc, char* argv[]) {
  WzResource::Init();
  Freetype::Init();

  if (argc == 3) {
    ServerMain::ServerInit(argv[1], SDL_atoi(argv[2]));
  } else {
    ServerMain::ServerInit();
  }
  Window::Create("sdlMS", kLogicWidth, kLogicHeight, kWindowWidth,
                 kWindowHeight);
  CameraGameInstance::Load(0, 0, kLogicWidth, kLogicHeight);
  KeyboardGameInstance::Load();
  AudioGameInstance::Init();
  // ShaderGameInstance::Init();

  CharacterGameInstance::InitCharacterBone();
  CharacterGameInstance::InitDefaultClothes();
  CharacterGameInstance::LoadSelfCharacter();
  CharacterGameInstance::self_.job = u"000";
  CharacterGameInstance::self_.level = 100;
  CharacterGameInstance::self_.name = u"进击的蓝蘑菇";
  // CharacterGameInstance::self_.morph = u"0001";

  SDL_HideCursor();
  CharacterSaveData cs;
  PackageGameInstance::Load(cs);
  CharacterStatGameInstance::Load(cs);
  QuestGameInstance::Load(cs);

  uint32_t start_map = 1;
  SceneSystemInstance::EnterPrepare(start_map, u"sp", 0);
  // SceneSystemInstance::prepare_pos = {600, -470};
  // CharacterCreateSystemInstance::Enter();
  // LoginSystemInstance::Enter();

  return SDL_APP_CONTINUE;
}

SDL_AppResult AppEvent(void* appstate, SDL_Event* event) {
  auto result =
      (event->type == SDL_EVENT_QUIT) ? SDL_APP_SUCCESS : SDL_APP_CONTINUE;
  for (auto& fn : System::event_systems_) {
    if (fn(event) == false) {
      break;
    }
  }
  return result;
}

void AppQuit(void* appstate, SDL_AppResult result) {
  GameSaveSystemInstance::SaveGame();
  SDL_Quit();
}

}  // namespace sdlms

// SDL entry points. These have C linkage and must stay at global scope, so
// they only forward to the namespaced implementations above.

SDL_AppResult SDL_AppIterate(void* appstate) {
  return sdlms::AppIterate(appstate);
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
  return sdlms::AppInit(appstate, argc, argv);
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
  return sdlms::AppEvent(appstate, event);
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
  sdlms::AppQuit(appstate, result);
}
