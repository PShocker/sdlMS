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
  static void Reset();

  static void Load(const CharacterSaveData &cs);
  static void Load();
  static inline std::flat_map<SDL_Scancode, GameInput> data;
  static SDL_Scancode FindKeyByVal(const std::string &target_val);
  static GameInput FindInputByCursor(int i);
  static std::optional<GameInput> LoadCursorInput();

  static void InsertInput(SDL_Scancode c, GameInput i);
  static void EraseInput(SDL_Scancode c);
  static void EraseInput(GameInput i);
};