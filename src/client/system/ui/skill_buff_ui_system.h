#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/game/game_skill.h"
#include <cstdint>
#include <flat_map>
#include <string>

class skill_buff_ui_system {
private:
  static void render_ui();
  static void render_ui(game_skill &sk, float x, float y);
  static void render_info();

public:
  static bool render_ui_info();
  static bool render();
  static bool event(SDL_Event *event);
};