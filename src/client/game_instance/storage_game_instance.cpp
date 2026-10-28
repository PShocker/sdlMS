#include "storage_game_instance.h"

void StorageGameInstance::Load(const GameSaveData &gs) {
  storage = gs.storage;
  return;
}