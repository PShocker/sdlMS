#include "keyboard_input_system.h"
#include "SDL3/SDL_events.h"
#include "input_system.h"
#include "src/client/game_instance/keyboard_game_instance.h"
#include <cstdio>

bool keyboard_input_system::event(SDL_Event *event) {
  if (event->type == SDL_EVENT_KEY_DOWN) {
    auto repeat = event->key.repeat;
    auto scan_code = event->key.scancode;
    if (keyboard_game_instance::data.contains(scan_code)) {
      auto &g_input = keyboard_game_instance::data.at(scan_code);
      if (g_input.type == "item" || g_input.type == "ui") {
        // 只有当repeat为false,才触发
        if (repeat == false) {
          if (g_input.type == "ui") {
            input_system::handle_ui_input(g_input);
          } else {
            input_system::handle_item_input(g_input);
          }
        }
      } else {
      }
    }
    printf("%d\n", scan_code);
  }
  return true;
}