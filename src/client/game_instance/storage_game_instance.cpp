#include "storage_game_instance.h"

void StorageGameInstance::Load(const GameSaveData &gs) {
  storage_ = gs.storage;
  return;
}