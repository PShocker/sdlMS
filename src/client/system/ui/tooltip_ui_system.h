#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/game/game_equip.h"
#include "src/client/game/game_install.h"
#include "src/client/game/game_item.h"
#include <cstdint>
#include <string>
class tooltip_ui_system {
public:
  static void render_equip(game_equip &equip, float x, float y);
  static void render_item(game_item &item, float x, float y);
  static void render_install(game_install &ins, float x, float y);
  static void render_skill(std::u16string id, uint8_t level, float x, float y);
  static void render_world_map_info(uint32_t id, float x, float y);
  static bool event(SDL_Event *event);
};