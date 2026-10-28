#include "game_instance/character_game_instance.h"
#include "game_instance/character_stat_game_instance.h"
#include "game_instance/keyboard_game_instance.h"
#include "game_instance/quest_game_instance.h"
#include "game_instance/shader_game_instance.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/shader_game_instance.h"
#include "src/client/system/system.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_main.h"
#include "src/server/server_system/server_system.h"
#include "system_instance/game_save_system_instance.h"
#include "system_instance/login_system_instance.h"
#include "system_instance/scene_system_instance.h"
#include "window/window.h"
#include <cstdint>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

SDL_AppResult SDL_AppIterate(void *appstate) {
  Window::Tick();
  Window::Clear();
  for (const auto &fn : System::logic_systems) {
    if (!fn()) {
      break;
    }
  }

  // shader_game_instance::start(shader_enum::grayscale);
  for (const auto &fn : System::render_systems) {
    if (!fn()) {
      break;
    }
  }
  // shader_game_instance::end();

  Window::Update();
  for (const auto &fn : ServerSystem::server_systems) {
    if (fn() == false) {
      break;
    }
  }
  return SDL_APP_CONTINUE;
}

// 逻辑分辨率
static int32_t logic_w = 1366;
static int32_t logic_h = 768;

// 窗口分辨率
static int32_t window_w = 1366;
static int32_t window_h = 768;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  WzResource::Init();
  Freetype::Init();

  if (argc == 3) {
    ServerMain::ServerInit(argv[1], SDL_atoi(argv[2]));
  } else {
    ServerMain::ServerInit();
  }
  Window::Create("sdlMS", logic_w, logic_h, window_w, window_h);
  CameraGameInstance::Load(0, 0, logic_w, logic_h);
  KeyboardGameInstance::Load();
  AudioGameInstance::Init();
  // shader_game_instance::init();

  CharacterGameInstance::InitCharacterBone();
  CharacterGameInstance::InitDefaultClothes();
  CharacterGameInstance::LoadSelfCharacter();
  CharacterGameInstance::self.job = u"000";
  CharacterGameInstance::self.level = 1;
  CharacterGameInstance::self.name = u"进击的蓝蘑菇";
  // character_game_instance::self.morph = u"0001";

  SDL_HideCursor();
  CharacterSaveData cs;
  PackageGameInstance::Load(cs);
  CharacterStatGameInstance::Load(cs);
  QuestGameInstance::Load(cs);

  SceneSystemInstance::EnterPrepare(1010, u"sp", 0);
  // scene_system_instance::prepare_pos = {600, -470};
  // character_create_system_instance::enter();
  // login_system_instance::enter();

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *Event) {
  auto r = (Event->type == SDL_EVENT_QUIT) ? SDL_APP_SUCCESS : SDL_APP_CONTINUE;
  for (auto &fn : System::event_systems) {
    if (fn(Event) == false) {
      break;
    }
  }
  return r;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  // save
  GameSaveSystemInstance::SaveGame();
  SDL_Quit();
}