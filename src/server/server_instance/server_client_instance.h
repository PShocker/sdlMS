#pragma once

#include "src/server/server/server_client.h"
#include <cstdint>
#include <flat_map>

class server_client_instance {
public:
  static void client_exit(uint64_t client_id);
  static inline std::flat_map<uint64_t, server_client> clients;
};