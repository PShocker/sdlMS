#pragma once

#include "SDL3/SDL_scancode.h"
#include "src/client/game/game_input.h"
#include "src/client/game/game_save.h"
#include <cstdint>
#include <flat_map>
#include <optional>
#include <string>
class KeyboardGameInstance {
public:
  static void reset();

  static void load(const CharacterSaveData &cs);
  static void load();
  static inline std::flat_map<SDL_Scancode, GameInput> data;
  static SDL_Scancode find_key_by_val(const std::string &target_val);
  static GameInput find_input_by_cursor(int i);
  static std::optional<GameInput> load_cursor_input();

  static void insert_input(SDL_Scancode c, GameInput i);
  static void erase_input(SDL_Scancode c);
  static void erase_input(GameInput i);
};