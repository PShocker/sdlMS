#include "storage_game_instance.h"

void storage_game_instance::load(const game_save &gs) {
  storage = gs.storage;
  return;
}