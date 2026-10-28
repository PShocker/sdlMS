#ifndef SDLMS_SRC_CLIENT_GAME_INSTANCE_GAIN_LOG_GAME_INSTANCE_H_
#define SDLMS_SRC_CLIENT_GAME_INSTANCE_GAIN_LOG_GAME_INSTANCE_H_

#include "src/client/game/game_gain_log.h"

#include <cstdint>
#include <string>
#include <vector>

namespace sdlms {

class GainLogGameInstance {
 public:
  static inline std::vector<GameGainLog> data_;

  static void Add(const std::u16string& id, int num, GainEnum type);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_INSTANCE_GAIN_LOG_GAME_INSTANCE_H_
