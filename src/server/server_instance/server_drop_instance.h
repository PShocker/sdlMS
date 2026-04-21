#pragma once

#include "src/server/server/server_drop.h"
#include <flat_map>

class server_map_drop {
  std::flat_map<int32_t, server_drop> dps;
};

class server_drop_instance {
public:
  static inline std::flat_map<uint32_t, server_map_drop>
      common_map_drop; // 普通
  static inline std::flat_multimap<uint32_t, server_map_drop>
      party_map_drop; // 组队
};