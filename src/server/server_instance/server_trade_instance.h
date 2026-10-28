#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/server.h"
namespace sdlms {

using namespace fbs;

class ServerTradeInstance {
public:
  static void HandleClient(uint64_t client_id, ClientCharacterTradeT &r);
  static void HandleServer(uint64_t client_id, ServerCharacterTradeT &r);
};

}  // namespace sdlms
