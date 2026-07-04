#include "game_instance/character_game_instance.h"
#include "game_instance/keyboard_game_instance.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
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
  window::tick();
  window::clear();
  for (const auto &fn : system::logic_systems) {
    if (!fn()) {
      break;
    }
  }
  for (const auto &fn : system::render_systems) {
    if (!fn()) {
      break;
    }
  }
  window::update();
  for (const auto &fn : server_system::server_systems) {
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
  wz_resource::init();
  freetype::init();

  if (argc == 3) {
    server_main::server_init(argv[1], SDL_atoi(argv[2]));
  } else {
    server_main::server_init();
  }
  window::create("sdlMS", logic_w, logic_h, window_w, window_h);
  camera_game_instance::load(0, 0, logic_w, logic_h);
  keyboard_game_instance::load();
  audio_game_instance::init();

  character_game_instance::init_character_bone();
  character_game_instance::load_self_character();

  SDL_HideCursor();
  scene_system_instance::enter_prepare(40, u"sp", 0);
  scene_system_instance::prepare_pos = {1527, 188};
  character_game_instance::self.pos = {1527, 188};
  // chatacter_create_system_instance::enter();
  // login_system_instance::enter();

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  auto r = (event->type == SDL_EVENT_QUIT) ? SDL_APP_SUCCESS : SDL_APP_CONTINUE;
  for (auto &fn : system::event_systems) {
    if (fn(event) == false) {
      break;
    }
  }
  return r;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  // save
  game_save_system_instance::save_game();
  SDL_Quit();
}