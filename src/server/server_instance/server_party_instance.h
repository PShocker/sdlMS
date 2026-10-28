#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/server.h"
#include "src/server/server/server_party.h"
#include <cstdint>
#include <flat_map>
#include <optional>
namespace sdlms {


using namespace fbs;

class ServerPartyInstance {
private:
  static ServerParty &LoadParty(uint64_t captain_id);

public:
  static inline std::flat_map<uint64_t, ServerParty> data_;

  static std::optional<ServerParty> LoadClientParty(uint64_t client_id);

  static void HandleParty(uint64_t client_id, ClientCharacterPartyT &r);
  static void HandleServerParty(uint64_t client_id, ServerCharacterPartyT &r);
};

}  // namespace sdlms
