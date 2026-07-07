#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/server.h"
using namespace fbs;

class server_trade_instance {
public:
  static void handle_client(uint64_t client_id, ClientCharacterTradeT &r);
  static void handle_server(uint64_t client_id, ServerCharacterTradeT &r);
};