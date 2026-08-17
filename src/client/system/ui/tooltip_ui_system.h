#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/game/game_item.h"
#include <cstdint>
#include <string>
class tooltip_ui_system {
private:
  static void render_backgrnd(float x, float y, float w, float h);
  static void render_equip_req(uint32_t req, uint32_t val,
                               const std::u16string &path, float x, float y);
  static void render_equip_job(const std::u16string &id, float x, float y);
  static void render_equip_bottom_inc(std::u16string label, std::u16string val,
                                      float x, float &y);

  static void render_equip_bottom(game_equip_item &equip, float x, float y);
  static float load_equip_bottom_h(game_equip_item &equip);

  static void render_deco_bottom(game_deco_item &deco, float x, float y);
  static float load_deco_bottom_h(const std::u16string &id);

  static void render_skill_bottom(const std::u16string &id, uint8_t level,
                                  float x, float y);
  static float load_skill_bottom_h(const std::u16string &id);

public:
  static void render_deco(game_deco_item &deco, float x, float y);
  static void render_equip(game_equip_item &equip, float x, float y);
  static void render_item(game_item &item, float x, float y);
  static void render_install(game_install_item &ins, float x, float y);
  static void render_skill(const std::u16string &id, uint8_t level, float x,
                           float y);
  static void render_world_map_info(uint32_t id, float x, float y);
  static bool event(SDL_Event *event);
};