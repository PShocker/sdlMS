#pragma once

#include "SDL3/SDL_scancode.h"
#include "src/client/game/game_input.h"
#include "src/client/game/game_save.h"
#include <cstdint>
#include <flat_map>
#include <optional>
#include <string>
class keyboard_game_instance {
private:
  static void reset();

public:
  static void load(const character_save &cs);
  static void load();
  static inline std::flat_map<SDL_Scancode, game_input> data;
  static SDL_Scancode find_key_by_val(const std::string &target_val);
  static game_input find_input_by_cursor(int i);
  static std::optional<game_input> load_cursor_input();

  static void insert_input(SDL_Scancode c, game_input i);
  static void erase_input(SDL_Scancode c);
  static void erase_input(game_input i);
};