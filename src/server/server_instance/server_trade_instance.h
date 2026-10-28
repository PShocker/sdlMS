#ifndef SDLMS_SRC_SERVER_SERVER_INSTANCE_SERVER_TRADE_INSTANCE_H_
#define SDLMS_SRC_SERVER_SERVER_INSTANCE_SERVER_TRADE_INSTANCE_H_

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/server.h"
namespace sdlms {


class ServerTradeInstance {
 public:
  static void HandleClient(uint64_t client_id, fbs::ClientCharacterTradeT& r);
  static void HandleServer(uint64_t client_id, fbs::ServerCharacterTradeT& r);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_SERVER_SERVER_INSTANCE_SERVER_TRADE_INSTANCE_H_
