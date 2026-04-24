#include "keyboard_game_instance.h"
#include "SDL3/SDL_scancode.h"
#include "src/client/game/game_input.h"

void keyboard_game_instance::reset() {
  data.clear();

  data[SDL_SCANCODE_UP] = {
      .type = "action",
      .val = "up",
  };
  data[SDL_SCANCODE_DOWN] = {
      .type = "action",
      .val = "down",
  };
  data[SDL_SCANCODE_LEFT] = {
      .type = "action",
      .val = "left",
  };
  data[SDL_SCANCODE_RIGHT] = {
      .type = "action",
      .val = "right",
  };
  data[SDL_SCANCODE_W] = {
      .type = "ui",
      .val = "worldMap",
  };
  data[SDL_SCANCODE_I] = {
      .type = "ui",
      .val = "package",
  };
}

void keyboard_game_instance::load() { reset(); }