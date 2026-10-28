#ifndef SDLMS_SRC_CLIENT_GAME_INSTANCE_LADDERROPE_GAME_INSTANCE_H_
#define SDLMS_SRC_CLIENT_GAME_INSTANCE_LADDERROPE_GAME_INSTANCE_H_

#include "src/client/game/game_ladderrope.h"
#include <flat_map>
#include <vector>
namespace sdlms {

class LadderropeGameInstance {
public:
  static void Load(uint32_t map_id);
  static inline std::flat_map<int32_t, GameLadderrope> data_;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_INSTANCE_LADDERROPE_GAME_INSTANCE_H_
