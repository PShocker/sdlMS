#include "keyboard_game_instance.h"
#include "SDL3/SDL_scancode.h"
#include "src/client/game/game_input.h"

void keyboard_game_instance::reset() {
  data = {
      {SDL_SCANCODE_UP, {.type = "action", .val = "up"}},
      {SDL_SCANCODE_DOWN, {.type = "action", .val = "down"}},
      {SDL_SCANCODE_LEFT, {.type = "action", .val = "left"}},
      {SDL_SCANCODE_RIGHT, {.type = "action", .val = "right"}},
      {SDL_SCANCODE_LALT, {.type = "action", .val = "jump"}},
      {SDL_SCANCODE_LCTRL, {.type = "action", .val = "attack"}},
      {SDL_SCANCODE_X, {.type = "action", .val = "sit"}},
      {SDL_SCANCODE_Z, {.type = "action", .val = "pick"}},
      {SDL_SCANCODE_W, {.type = "ui", .val = "worldMap"}},
      {SDL_SCANCODE_I, {.type = "ui", .val = "package"}},
      {SDL_SCANCODE_S, {.type = "ui", .val = "characterStat"}},
      // {SDL_SCANCODE_O, {.type = "ui", .val = "keyBind"}},
      {SDL_SCANCODE_K, {.type = "ui", .val = "skill"}},
      {SDL_SCANCODE_E, {.type = "ui", .val = "equip"}},
      {SDL_SCANCODE_C, {.type = "ui", .val = "craft"}},
      {SDL_SCANCODE_Q, {.type = "ui", .val = "quest"}},
      {SDL_SCANCODE_P, {.type = "ui", .val = "option"}},
      {SDL_SCANCODE_A, {.type = "skill", .val = "4001003"}},
      {SDL_SCANCODE_V, {.type = "skill", .val = "0001000"}},
      {SDL_SCANCODE_B, {.type = "skill", .val = "4001001"}},
      {SDL_SCANCODE_N, {.type = "skill", .val = "0001002"}},
      {SDL_SCANCODE_H, {.type = "skill", .val = "2001003"}},
      {SDL_SCANCODE_C, {.type = "item", .val = "03010008"}},
      {SDL_SCANCODE_F4, {.type = "face", .val = "vomit"}},

  };
}

void keyboard_game_instance::load() { reset(); }

void keyboard_game_instance::load(const character_save &cs) {
  data = {};
  for (auto &k : cs.keys) {
    data[(SDL_Scancode)k.scan_code] = {
        .type = k.type,
        .val = k.val,
        .sub_val = k.sub_val,
    };
  }
}

SDL_Scancode
keyboard_game_instance::find_key_by_val(const std::string &target_val) {
  for (const auto &[scancode, binding] : data) {
    if (binding.val == target_val) {
      return scancode;
    }
  }
  return SDL_SCANCODE_UNKNOWN; // 未找到
}