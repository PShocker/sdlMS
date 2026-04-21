#pragma once

#include "src/server/server/server_foothold.h"
#include <flat_map>

class server_map_foothold {
public:
  std::flat_map<int32_t, server_foothold> fhs;
};

class server_foothold_instance {
public:
  static inline void load_server_fh(uint32_t map_id);
  static inline std::flat_map<uint32_t, server_map_foothold> map_fh;
};