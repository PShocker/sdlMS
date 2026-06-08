#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/server/server/server_drop.h"
#include <flat_map>

class server_drop_instance {
public:
  static void handle_drop(uint64_t client_id, fbs::ClientCharacterDropT &r);
};