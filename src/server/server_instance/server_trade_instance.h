#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/server.h"
namespace sdlms {


class ServerTradeInstance {
public:
  static void HandleClient(uint64_t client_id, fbs::ClientCharacterTradeT &r);
  static void HandleServer(uint64_t client_id, fbs::ServerCharacterTradeT &r);
};

}  // namespace sdlms
