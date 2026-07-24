#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/server.h"
#include "src/server/server/server_party.h"
#include <cstdint>
#include <flat_map>
#include <optional>

using namespace fbs;

class server_party_instance {
private:
  static server_party &load_party(uint64_t captain_id);

public:
  static inline std::flat_map<uint64_t, server_party> data;

  static std::optional<server_party> load_client_party(uint64_t client_id);

  static void handle_party(uint64_t client_id, ClientCharacterPartyT &r);
  static void handle_server_party(uint64_t client_id, ServerCharacterPartyT &r);
};