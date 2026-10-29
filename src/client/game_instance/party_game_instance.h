#pragma once

#include "src/client/game/game_party.h"

#include <optional>

namespace sdlms {

class PartyGameInstance {
 public:
  static inline std::optional<GameParty> data_;
};

}  // namespace sdlms
