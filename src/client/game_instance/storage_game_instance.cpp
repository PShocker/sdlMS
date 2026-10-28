#include "storage_game_instance.h"

void StorageGameInstance::load(const GameSaveData &gs) {
  storage = gs.storage;
  return;
}