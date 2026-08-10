#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_character.h"
#include <cstdint>
#include <optional>
#include <string>
class scene_system_instance {
public:
  static inline uint32_t map_id;
  static inline uint32_t prepare_map_id;
  static inline std::optional<SDL_FPoint> prepare_pos;
  static void enter(uint32_t map_id);
  static void enter_fade();
  static void enter_prepare(uint32_t map_id, const std::u16string &pn, int i);
  static void enter_quit();
  static void quit_prepare();
  static void enter_cash_shop();
  static void cash_shop_prepare();

private:
  static SDL_FPoint load_prepare_pos(const std::u16string &pn, uint8_t index);
  static void enter_prepare();
  static bool render_game();
};