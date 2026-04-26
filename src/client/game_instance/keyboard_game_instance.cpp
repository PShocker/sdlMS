#include "keyboard_game_instance.h"
#include "SDL3/SDL_scancode.h"
#include "src/client/game/game_input.h"

void keyboard_game_instance::reset() {
  data = {
      {SDL_SCANCODE_UP, {.type = "action", .val = "up"}},
      {SDL_SCANCODE_DOWN, {.type = "action", .val = "down"}},
      {SDL_SCANCODE_LEFT, {.type = "action", .val = "left"}},
      {SDL_SCANCODE_RIGHT, {.type = "action", .val = "right"}},
      {SDL_SCANCODE_W, {.type = "ui", .val = "worldMap"}},
      {SDL_SCANCODE_I, {.type = "ui", .val = "package"}},
      {SDL_SCANCODE_S, {.type = "ui", .val = "characterStat"}},
      {SDL_SCANCODE_O, {.type = "ui", .val = "keyBind"}},
      {SDL_SCANCODE_K, {.type = "ui", .val = "skill"}},
      {SDL_SCANCODE_L, {.type = "ui", .val = "characterInfo"}},
      {SDL_SCANCODE_E, {.type = "ui", .val = "equip"}},
      {SDL_SCANCODE_C, {.type = "ui", .val = "craft"}},
  };
}

void keyboard_game_instance::load() { reset(); }