#include "src/common/request/client_request.h"
#include "src/server/server_main.h"
#include "window/window.h"
#include <cstdint>
#include <cstdio>


#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

SDL_AppResult SDL_AppIterate(void *appstate) {
  window::tick();
  window::clear();
  return SDL_APP_CONTINUE;
}

int32_t width = 800;
int32_t height = 600;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  if (argc == 3) {
    server_main::server_init(argv[1], SDL_atoi(argv[2]));
  } else {
    server_main::server_init();
  }

  SDL_HideCursor();

  client_request::client_scene_request(true, 1000);

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  auto r = (event->type == SDL_EVENT_QUIT) ? SDL_APP_SUCCESS : SDL_APP_CONTINUE;

  return r;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  printf("exit");
  SDL_Quit();
}