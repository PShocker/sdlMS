#pragma once

#include "src/client/game/game_input.h"
class input_system {
public:
  static void handle_ui_input(game_input &g_input);
  static void handle_item_input(game_input &g_input);
  static void handle_action_input(game_input &g_input, bool press);
  static void handle_skill_input(game_input &g_input, bool press);
};