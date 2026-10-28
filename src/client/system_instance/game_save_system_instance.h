#pragma once
#include "src/client/game/game_save.h"
#include <optional>
#include <string>

class GameSaveSystemInstance {
public:
  static inline GameSaveData save;
  static bool load_save(const std::string &login);

  static bool save_game();
};