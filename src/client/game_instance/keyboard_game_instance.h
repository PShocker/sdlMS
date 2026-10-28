#ifndef SDLMS_SRC_CLIENT_GAME_INSTANCE_KEYBOARD_GAME_INSTANCE_H_
#define SDLMS_SRC_CLIENT_GAME_INSTANCE_KEYBOARD_GAME_INSTANCE_H_

#include <SDL3/SDL_scancode.h>
#include "src/client/game/game_input.h"
#include "src/client/game/game_save.h"
#include <cstdint>
#include <flat_map>
#include <optional>
#include <string>
namespace sdlms {

class KeyboardGameInstance {
public:
  static void Reset();

  static void Load(const CharacterSaveData &cs);
  static void Load();
  static inline std::flat_map<SDL_Scancode, GameInput> data_;
  static SDL_Scancode FindKeyByVal(const std::string &target_val);
  static GameInput FindInputByCursor(int i);
  static std::optional<GameInput> LoadCursorInput();

  static void InsertInput(SDL_Scancode c, GameInput i);
  static void EraseInput(SDL_Scancode c);
  static void EraseInput(GameInput i);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_INSTANCE_KEYBOARD_GAME_INSTANCE_H_
