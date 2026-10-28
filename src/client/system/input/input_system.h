#pragma once

#include "src/client/game/game_input.h"
class InputSystem {
public:
  static void handle_ui_input(GameInput &g_input);
  static void handle_item_input(GameInput &g_input, bool press);
  static void handle_action_input(GameInput &g_input, bool press);
  static void handle_skill_input(GameInput &g_input, bool press);
  static void handle_face_input(GameInput &g_input, bool press);
  static void reset();
};