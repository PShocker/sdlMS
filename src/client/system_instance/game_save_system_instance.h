#pragma once
#include "src/client/game/game_save.h"
#include <optional>
#include <string>

class GameSaveSystemInstance {
public:
  static inline GameSaveData save_;
  static bool LoadSave(const std::string &login);

  static bool SaveGame();
};