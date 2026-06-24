#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/game/game_animate.h"
#include "src/client/game/game_character.h"
#include <array>
#include <cstdint>
#include <optional>
#include <vector>

class character_choose_ui_system {
private:
  static bool new_animate();
  static bool back_animate();

  static SDL_FPoint load_pos();
  static void render_backgrnd();
  static void render_button();
  static void render_effect();
  static void render_banner();
  static void render_character();
  static void render_character_board(uint8_t i);
  static void render_character_effect(uint8_t i);

  static void event_button_delete_cb();

  static void event_button_select();
  static void event_button_new();
  static void event_button_delete();
  static void event_button_back();

  static bool event_button(SDL_Event *event);
  static bool event_choose_character(SDL_Event *event);

public:
  static inline std::optional<uint8_t> choose;
  static inline std::vector<game_character> characters;
  static inline game_animate board;
  static inline game_animate effect;

  static bool render();
  static bool event(SDL_Event *event);
  static bool run();
};