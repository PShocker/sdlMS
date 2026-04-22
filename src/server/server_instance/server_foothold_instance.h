#pragma once

#include "src/server/server/server_foothold.h"
#include <flat_map>

class server_foothold_instance {
public:
  static inline std::flat_map<int32_t, server_foothold>
  load_server_fh(uint32_t map_id);
};