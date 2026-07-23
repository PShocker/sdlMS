#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/server.h"
#include "src/server/server/server_party.h"
#include <cstdint>
#include <flat_map>

using namespace fbs;

class server_party_instance {
public:
  static inline std::flat_map<uint64_t, server_party> data;

  static void handle_party(uint64_t client_id, ClientCharacterPartyT &r);
  static void handle_server_party(uint64_t client_id, ServerCharacterPartyT &r);
};