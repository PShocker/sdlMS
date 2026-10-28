#include "src/client/game_instance/storage_game_instance.h"
namespace sdlms {


void StorageGameInstance::Load(const GameSaveData& gs) {
  storage_ = gs.storage;
  return;
}

}  // namespace sdlms
