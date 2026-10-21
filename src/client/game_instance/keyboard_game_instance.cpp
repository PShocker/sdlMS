#include "keyboard_game_instance.h"
#include "SDL3/SDL_scancode.h"
#include "cursor_game_instance.h"
#include "src/client/game/game_input.h"
#include "src/client/game_instance/package_game_instance.h"
#include <format>
#include <optional>
#include <string>

void keyboard_game_instance::reset() {
  data.clear();
  insert_input(SDL_SCANCODE_ESCAPE, {.type = "ui", .val = "menu"});
  insert_input(SDL_SCANCODE_F1, {.type = "face", .val = "hit"});
  insert_input(SDL_SCANCODE_F2, {.type = "face", .val = "smile"});
  insert_input(SDL_SCANCODE_F3, {.type = "face", .val = "troubled"});
  insert_input(SDL_SCANCODE_F4, {.type = "face", .val = "cry"});
  insert_input(SDL_SCANCODE_F5, {.type = "face", .val = "angry"});
  insert_input(SDL_SCANCODE_F6, {.type = "face", .val = "bewildered"});
  insert_input(SDL_SCANCODE_F7, {.type = "face", .val = "stunned"});
  insert_input(SDL_SCANCODE_1, {.type = "ui", .val = "toAll"});
  insert_input(SDL_SCANCODE_2, {.type = "ui", .val = "toParty"});
  insert_input(SDL_SCANCODE_3, {.type = "ui", .val = "toBuddy"});
  insert_input(SDL_SCANCODE_Q, {.type = "ui", .val = "quest"});
  insert_input(SDL_SCANCODE_W, {.type = "ui", .val = "worldMap"});
  insert_input(SDL_SCANCODE_E, {.type = "ui", .val = "equip"});
  insert_input(SDL_SCANCODE_I, {.type = "ui", .val = "package"});
  insert_input(SDL_SCANCODE_O, {.type = "ui", .val = "buddy"});
  insert_input(SDL_SCANCODE_P, {.type = "ui", .val = "party"});
  insert_input(SDL_SCANCODE_LEFTBRACKET, {.type = "ui", .val = "mapleChat"});
  insert_input(SDL_SCANCODE_RIGHTBRACKET, {.type = "ui", .val = "quickSlot"});
  insert_input(SDL_SCANCODE_BACKSLASH, {.type = "ui", .val = "keyBind"});
  insert_input(SDL_SCANCODE_S, {.type = "ui", .val = "characterStat"});
  insert_input(SDL_SCANCODE_H, {.type = "ui", .val = "whisper"});
  insert_input(SDL_SCANCODE_K, {.type = "ui", .val = "skill"});
  insert_input(SDL_SCANCODE_L, {.type = "ui", .val = "craft"});
  insert_input(SDL_SCANCODE_SEMICOLON, {.type = "ui", .val = "mailBox"});
  insert_input(SDL_SCANCODE_APOSTROPHE, {.type = "ui", .val = "chat+"});
  insert_input(SDL_SCANCODE_Z, {.type = "action", .val = "pick"});
  insert_input(SDL_SCANCODE_X, {.type = "action", .val = "sit"});
  insert_input(SDL_SCANCODE_M, {.type = "ui", .val = "miniMap"});
  insert_input(SDL_SCANCODE_COMMA, {.type = "ui", .val = "changeChannel"});
  insert_input(SDL_SCANCODE_LCTRL, {.type = "action", .val = "attack"});
  insert_input(SDL_SCANCODE_LALT, {.type = "action", .val = "jump"});
  insert_input(SDL_SCANCODE_SPACE, {.type = "action", .val = "harvest"});
  insert_input(SDL_SCANCODE_UP, {.type = "action", .val = "up"});
  insert_input(SDL_SCANCODE_DOWN, {.type = "action", .val = "down"});
  insert_input(SDL_SCANCODE_LEFT, {.type = "action", .val = "left"});
  insert_input(SDL_SCANCODE_RIGHT, {.type = "action", .val = "right"});
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
  case cursor_game_instance::keybind_action_sit: {
    input = {
        .type = "action",
        .val = "sit",
    };
    break;
  }
  case cursor_game_instance::keybind_action_jump: {
    input = {
        .type = "action",
        .val = "jump",
    };
    break;
  }
  case cursor_game_instance::keybind_action_pick: {
    input = {
        .type = "action",
        .val = "pick",
    };
    break;
  }
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
        .val = "characterStat",
    };
    break;
  }
  case cursor_game_instance::keybind_ui_keyBind: {
    input = {
        .type = "ui",
        .val = "keyBind",
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
  case cursor_game_instance::keybind_ui_buddy: {
    input = {
        .type = "ui",
        .val = "buddy",
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
  case cursor_game_instance::keybind_ui_menu: {
    input = {
        .type = "ui",
        .val = "menu",
    };
    break;
  }
  case cursor_game_instance::keybind_face_hit: {
    input = {
        .type = "face",
        .val = "hit",
    };
    break;
  }
  case cursor_game_instance::keybind_face_smile: {
    input = {
        .type = "face",
        .val = "smile",
    };
    break;
  }
  case cursor_game_instance::keybind_face_troubled: {
    input = {
        .type = "face",
        .val = "troubled",
    };
    break;
  }
  case cursor_game_instance::keybind_face_cry: {
    input = {
        .type = "face",
        .val = "cry",
    };
    break;
  }
  case cursor_game_instance::keybind_face_angry: {
    input = {
        .type = "face",
        .val = "angry",
    };
    break;
  }
  case cursor_game_instance::keybind_face_bewildered: {
    input = {
        .type = "face",
        .val = "bewildered",
    };
    break;
  }
  case cursor_game_instance::keybind_face_stunned: {
    input = {
        .type = "face",
        .val = "stunned",
    };
    break;
  }
  case cursor_game_instance::keybind_ui_toAll: {
    input = {
        .type = "ui",
        .val = "toAll",
    };
    break;
  }
  case cursor_game_instance::keybind_ui_toParty: {
    input = {
        .type = "ui",
        .val = "toParty",
    };
    break;
  }
  case cursor_game_instance::keybind_ui_party: {
    input = {
        .type = "ui",
        .val = "party",
    };
    break;
  }
  case cursor_game_instance::keybind_ui_mapleChat: {
    input = {
        .type = "ui",
        .val = "mapleChat",
    };
    break;
  }
  case cursor_game_instance::keybind_ui_quickSlot: {
    input = {
        .type = "ui",
        .val = "quickSlot",
    };
    break;
  }
  case cursor_game_instance::keybind_ui_whisper: {
    input = {
        .type = "ui",
        .val = "whisper",
    };
    break;
  }
  case cursor_game_instance::keybind_ui_mailBox: {
    input = {
        .type = "ui",
        .val = "mailBox",
    };
    break;
  }
  case cursor_game_instance::keybind_ui_chatPlus: {
    input = {
        .type = "ui",
        .val = "chat+",
    };
    break;
  }
  case cursor_game_instance::keybind_ui_miniMap: {
    input = {
        .type = "ui",
        .val = "miniMap",
    };
    break;
  }
  case cursor_game_instance::keybind_ui_changeChannel: {
    input = {
        .type = "ui",
        .val = "changeChannel",
    };
    break;
  }
  case cursor_game_instance::keybind_action_harvest: {
    input = {
        .type = "action",
        .val = "harvest",
    };
    break;
  }
  }
  return input;
}

void keyboard_game_instance::insert_input(SDL_Scancode c, game_input i) {
  erase_input(c);
  erase_input(i);
  switch (c) {
  case SDL_SCANCODE_LCTRL:
  case SDL_SCANCODE_RCTRL:
    data[SDL_SCANCODE_LCTRL] = i;
    data[SDL_SCANCODE_RCTRL] = i;
    break;
  case SDL_SCANCODE_LALT:
  case SDL_SCANCODE_RALT:
    data[SDL_SCANCODE_LALT] = i;
    data[SDL_SCANCODE_RALT] = i;
    break;
  case SDL_SCANCODE_LSHIFT:
  case SDL_SCANCODE_RSHIFT:
    data[SDL_SCANCODE_LSHIFT] = i;
    data[SDL_SCANCODE_RSHIFT] = i;
    break;
  default:
    data[c] = i;
    break;
  }
}

void keyboard_game_instance::erase_input(SDL_Scancode c) {
  switch (c) {
  case SDL_SCANCODE_LCTRL:
  case SDL_SCANCODE_RCTRL:
    data.erase(SDL_SCANCODE_LCTRL);
    data.erase(SDL_SCANCODE_RCTRL);
    break;
  case SDL_SCANCODE_LALT:
  case SDL_SCANCODE_RALT:
    data.erase(SDL_SCANCODE_LALT);
    data.erase(SDL_SCANCODE_RALT);
    break;
  case SDL_SCANCODE_LSHIFT:
  case SDL_SCANCODE_RSHIFT:
    data.erase(SDL_SCANCODE_LSHIFT);
    data.erase(SDL_SCANCODE_RSHIFT);
    break;
  default:
    data.erase(c);
    break;
  }
}

void keyboard_game_instance::erase_input(game_input i) {
  std::erase_if(data, [&](const auto &kv) {
    return kv.second == i; // 举例字段
  });
}

std::optional<game_input> keyboard_game_instance::load_cursor_input() {
  if (!cursor_game_instance::cursor_hand.has_value()) {
    return std::nullopt;
  }
  auto &cursor_hand = cursor_game_instance::cursor_hand.value();
  std::string id;
  switch (cursor_hand.type) {
  case cursor_game_instance::keybind: {
    if (keyboard_game_instance::data.contains((SDL_Scancode)cursor_hand.val)) {
      return keyboard_game_instance::data.at((SDL_Scancode)cursor_hand.val);
    } else {
      return find_input_by_cursor(cursor_hand.sub_val);
    }
    break;
  }
  case cursor_game_instance::package: {
    auto &item =
        package_game_instance::data[cursor_hand.val][cursor_hand.sub_val];
    id = std::string{item->id.begin(), item->id.end()};
    if (id.starts_with("02")) {
      if (id.starts_with("0204") || id.starts_with("0206") ||
          id.starts_with("0207")) {
        break;
      }
      game_input input{
          .type = "item",
          .val = id,
      };
      return input;
    }
    break;
  }
  case cursor_game_instance::skill: {
    id = std::format("{:07d}", cursor_hand.sub_val);
    game_input input{
        .type = "skill",
        .val = id,
    };
    return input;
    break;
  }
  default: {
    break;
  }
  }
  return std::nullopt;
}