#pragma once

#include "src/server/server/server_client.h"
#include <cstdint>
#include <flat_map>

class ServerClientInstance {
public:
  static void client_exit(uint64_t client_id);
  static inline std::flat_map<uint64_t, ServerClient> clients;
};