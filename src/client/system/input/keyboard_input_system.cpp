#include "src/client/system/input/keyboard_input_system.h"

#include "src/client/game_instance/keyboard_game_instance.h"
#include "src/client/system/input/input_system.h"

#include <SDL3/SDL_events.h>

#include <cstdio>
#include <flat_map>
#include <string>

namespace sdlms {

bool KeyboardInputSystem::Event(SDL_Event* event) {
  auto repeat = event->key.repeat;
  auto scan_code = event->key.scancode;
  if (!KeyboardGameInstance::data_.contains(scan_code)) {
    return true;
  }
  auto& g_input = KeyboardGameInstance::data_.at(scan_code);
  if (event->type == SDL_EVENT_KEY_DOWN) {
    if (repeat == false) {
      if (g_input.type == "item") {
        InputSystem::HandleItemInput(g_input, true);
      } else if (g_input.type == "ui") {
        InputSystem::HandleUiInput(g_input);
      } else if (g_input.type == "action") {
        InputSystem::HandleActionInput(g_input, true);
      } else if (g_input.type == "skill") {
        InputSystem::HandleSkillInput(g_input, true);
      } else if (g_input.type == "face") {
        InputSystem::HandleFaceInput(g_input, true);
      }
    }
  } else if (event->type == SDL_EVENT_KEY_UP) {
    if (g_input.type == "item") {
      InputSystem::HandleItemInput(g_input, false);
    } else if (g_input.type == "action") {
      InputSystem::HandleActionInput(g_input, false);
    } else if (g_input.type == "skill") {
      InputSystem::HandleSkillInput(g_input, false);
    } else if (g_input.type == "face") {
      InputSystem::HandleFaceInput(g_input, false);
    }
  }
  return false;
}

void KeyboardInputSystem::Reset() {
  InputSystem::Reset();
}

}  // namespace sdlms
