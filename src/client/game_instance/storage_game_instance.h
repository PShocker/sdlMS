#ifndef SDLMS_SRC_CLIENT_GAME_INSTANCE_STORAGE_GAME_INSTANCE_H_
#define SDLMS_SRC_CLIENT_GAME_INSTANCE_STORAGE_GAME_INSTANCE_H_

#include "src/client/game/game_save.h"
#include "src/client/game/game_storage.h"

namespace sdlms {

class StorageGameInstance {
 public:
  static inline GameStorage storage_;

  static void Load(const GameSaveData& gs);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_INSTANCE_STORAGE_GAME_INSTANCE_H_
