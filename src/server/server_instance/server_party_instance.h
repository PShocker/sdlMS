#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/server.h"
#include "src/server/server/server_party.h"
#include <cstdint>
#include <flat_map>
#include <optional>

using namespace fbs;

class ServerPartyInstance {
private:
  static ServerParty &load_party(uint64_t captain_id);

public:
  static inline std::flat_map<uint64_t, ServerParty> data;

  static std::optional<ServerParty> load_client_party(uint64_t client_id);

  static void handle_party(uint64_t client_id, ClientCharacterPartyT &r);
  static void handle_server_party(uint64_t client_id, ServerCharacterPartyT &r);
};