#pragma once

#include "src/client/game/game_save.h"
#include "src/client/game/game_storage.h"
class StorageGameInstance {
public:
  static inline GameStorage storage_;

  static void Load(const GameSaveData &gs);
};