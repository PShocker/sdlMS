#ifndef SDLMS_SRC_CLIENT_GAME_INSTANCE_PARTY_GAME_INSTANCE_H_
#define SDLMS_SRC_CLIENT_GAME_INSTANCE_PARTY_GAME_INSTANCE_H_
#include "src/client/game/game_party.h"
#include <optional>

namespace sdlms {

class PartyGameInstance {
public:
  static inline std::optional<GameParty> data_;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_INSTANCE_PARTY_GAME_INSTANCE_H_
