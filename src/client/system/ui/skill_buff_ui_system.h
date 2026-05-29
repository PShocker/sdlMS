#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/game/game_buff.h"
#include <cstdint>
#include <flat_map>
#include <string>

class skill_buff_ui_system {
private:
  static void render_ui(game_buff &buf, float x, float y);
  static void render_ui_info(game_buff &buf, float x, float y);

public:
  static void render();
  static bool event(SDL_Event *event);
};