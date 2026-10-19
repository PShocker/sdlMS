#include "keyboard_game_instance.h"
#include "SDL3/SDL_scancode.h"
#include "cursor_game_instance.h"
#include "src/client/game/game_input.h"
#include <string>

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
      {SDL_SCANCODE_A, {.type = "skill", .val = "3001002"}},
      {SDL_SCANCODE_LSHIFT, {.type = "skill", .val = "4001003"}},
      {SDL_SCANCODE_V, {.type = "skill", .val = "0001000"}},
      {SDL_SCANCODE_B, {.type = "skill", .val = "4001001"}},
      {SDL_SCANCODE_N, {.type = "skill", .val = "0001002"}},
      {SDL_SCANCODE_H, {.type = "skill", .val = "2001003"}},
      {SDL_SCANCODE_SPACE, {.type = "skill", .val = "2301000"}},
      {SDL_SCANCODE_M, {.type = "skill", .val = "3101003"}},
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

game_input keyboard_game_instance::find_input_by_cursor(int i) {
  game_input input;
  switch ((cursor_game_instance::cursor_keybind_type)i) {
  case cursor_game_instance::keybind_action_attack: {
    input = {
        .type = "action",
        .val = "attack",
    };
    break;
  }
  case cursor_game_instance::keybind_action_sit:
  case cursor_game_instance::keybind_action_jump:
  case cursor_game_instance::keybind_action_pick:
  case cursor_game_instance::keybind_action_up: {
    input = {
        .type = "action",
        .val = "up",
    };
    break;
  }
  case cursor_game_instance::keybind_action_down: {
    input = {
        .type = "action",
        .val = "down",
    };
    break;
  }
  case cursor_game_instance::keybind_action_left: {
    input = {
        .type = "action",
        .val = "left",
    };
    break;
  }
  case cursor_game_instance::keybind_action_right: {
    input = {
        .type = "action",
        .val = "right",
    };
    break;
  }
  case cursor_game_instance::keybind_ui_worldMap: {
    input = {
        .type = "ui",
        .val = "worldMap",
    };
    break;
  }
  case cursor_game_instance::keybind_ui_package: {
    input = {
        .type = "ui",
        .val = "package",
    };
    break;
  }
  case cursor_game_instance::keybind_ui_characterStat: {
    input = {
        .type = "ui",
        .val = "right",
    };
    break;
  }
  case cursor_game_instance::keybind_ui_keybind: {
    input = {
        .type = "ui",
        .val = "keybind",
    };
    break;
  }
  case cursor_game_instance::keybind_ui_skill: {
    input = {
        .type = "ui",
        .val = "skill",
    };
    break;
  }
  case cursor_game_instance::keybind_ui_equip: {
    input = {
        .type = "ui",
        .val = "equip",
    };
    break;
  }
  case cursor_game_instance::keybind_ui_craft: {
    input = {
        .type = "ui",
        .val = "craft",
    };
    break;
  }
  case cursor_game_instance::keybind_ui_quest: {
    input = {
        .type = "ui",
        .val = "quest",
    };
    break;
  }
  case cursor_game_instance::keybind_ui_option: {
    input = {
        .type = "ui",
        .val = "option",
    };
    break;
  }
  case cursor_game_instance::keybind_face_vomit: {
    input = {
        .type = "face",
        .val = "vomit",
    };
    break;
  }
  default: {
    return input;
  }
  }
  return input;
}

void keyboard_game_instance::add_input(SDL_Scancode c, input_type t,
                                       const std::string &id) {
  data.erase(c);
  if (id.empty()) {
    return;
  }
  std::string type;
  switch (t) {
  case none: {
    break;
  }
  case item: {
    type = "item";
    break;
  }
  case skill: {
    type = "skill";
    break;
  }
  }
  std::erase_if(data, [&](const auto &kv) {
    game_input input = kv.second;
    return input.type == type && input.val == id; // 举例字段
  });
  data[c].type = type;
  data[c].val = id;
}