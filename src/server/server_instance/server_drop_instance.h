#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/server/server/server_drop.h"
#include <cstdint>
#include <flat_map>

using namespace fbs;

class server_drop_instance {
private:
  static void save_drop(uint64_t map_id, const DropT &drop);
  static void handle_server_dt(const DropT &dt);

public:
  static void handle_drop(uint64_t client_id, ClientCharacterDropT &r);
  static void handle_pick(uint64_t client_id, ClientCharacterPickT &r);
  static void handle_server_drop(uint64_t client_id, ServerCharacterDropT &r);
};