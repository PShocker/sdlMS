#pragma once

#include "src/server/server/server_client.h"
#include <flat_map>

class server_client_instance {
public:
  static inline std::flat_map<uint64_t, server_client> clients;
};