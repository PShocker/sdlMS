#pragma once

#include "src/client/game/game_save.h"
#include "src/client/game/game_storage.h"
class storage_game_instance {
public:
  static inline game_storage storage;

  static void load(const game_save &gs);
};