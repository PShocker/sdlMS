#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/server/server/server_drop.h"
#include <cstdint>
#include <flat_map>

class server_drop_instance {
private:
  static void save_drop(uint64_t map_id, const DropT &drop);

public:
  static void handle_drop(uint64_t client_id, fbs::ClientCharacterDropT &r);
  static void handle_pick(uint64_t client_id, fbs::ClientCharacterPickT &r);
};