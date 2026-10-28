#ifndef SDLMS_SRC_CLIENT_GAME_INSTANCE_EFFECT_GAME_INSTANCE_H_
#define SDLMS_SRC_CLIENT_GAME_INSTANCE_EFFECT_GAME_INSTANCE_H_

#include "src/client/game/game_character.h"
#include "src/client/game/game_effect.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include <wz/Node.h>
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <string>
namespace sdlms {



class EffectGameInstance {
public:
  static inline std::array<std::vector<GameEffect>, 8> data_;
  static void Reset();
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_INSTANCE_EFFECT_GAME_INSTANCE_H_
