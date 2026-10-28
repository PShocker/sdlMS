#ifndef SDLMS_SRC_CLIENT_SYSTEM_INSTANCE_GAME_SAVE_SYSTEM_INSTANCE_H_
#define SDLMS_SRC_CLIENT_SYSTEM_INSTANCE_GAME_SAVE_SYSTEM_INSTANCE_H_

#include "src/client/game/game_save.h"

#include <optional>
#include <string>

namespace sdlms {

class GameSaveSystemInstance {
 public:
  static inline GameSaveData save_;
  static bool LoadSave(const std::string& login);

  static bool SaveGame();
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_INSTANCE_GAME_SAVE_SYSTEM_INSTANCE_H_
