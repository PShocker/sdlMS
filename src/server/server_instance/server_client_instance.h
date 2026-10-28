#pragma once

#include "src/server/server/server_client.h"
#include <cstdint>
#include <flat_map>
namespace sdlms {


class ServerClientInstance {
public:
  static void ClientExit(uint64_t client_id);
  static inline std::flat_map<uint64_t, ServerClient> clients_;
};

}  // namespace sdlms
