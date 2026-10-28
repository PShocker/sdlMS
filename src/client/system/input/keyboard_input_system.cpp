#include "keyboard_input_system.h"
#include "SDL3/SDL_events.h"
#include "input_system.h"
#include "src/client/game_instance/keyboard_game_instance.h"
#include <cstdio>
#include <flat_map>
#include <string>

bool KeyboardInputSystem::event(SDL_Event *event) {
  auto repeat = event->key.repeat;
  auto scan_code = event->key.scancode;
  if (!KeyboardGameInstance::data.contains(scan_code)) {
    return true;
  }
  auto &g_input = KeyboardGameInstance::data.at(scan_code);
  if (event->type == SDL_EVENT_KEY_DOWN) {
    if (repeat == false) {
      if (g_input.type == "item") {
        InputSystem::handle_item_input(g_input, true);
      } else if (g_input.type == "ui") {
        InputSystem::handle_ui_input(g_input);
      } else if (g_input.type == "action") {
        InputSystem::handle_action_input(g_input, true);
      } else if (g_input.type == "skill") {
        InputSystem::handle_skill_input(g_input, true);
      } else if (g_input.type == "face") {
        InputSystem::handle_face_input(g_input, true);
      }
    }
  } else if (event->type == SDL_EVENT_KEY_UP) {
    if (g_input.type == "item") {
      InputSystem::handle_item_input(g_input, false);
    } else if (g_input.type == "action") {
      InputSystem::handle_action_input(g_input, false);
    } else if (g_input.type == "skill") {
      InputSystem::handle_skill_input(g_input, false);
    } else if (g_input.type == "face") {
      InputSystem::handle_face_input(g_input, false);
    }
  }
  return false;
}

void KeyboardInputSystem::reset() {
  InputSystem::reset();
}