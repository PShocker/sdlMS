#pragma once

#include "src/server/server/server_mob.h"
#include <cstdint>
#include <flat_map>

class server_common_map_mob {
public:
  std::flat_map<uint32_t, server_mob> mobs;
};

class server_party_map_mob {
public:
  uint32_t party_id;
  std::flat_map<uint32_t, server_mob> mobs;
};

class server_mob_instance {
public:
  static inline void load_server_mob(uint32_t map_id);
  static inline void load_party_mob(uint32_t map_id, uint32_t party_id);
  static inline void load_summon_mob(uint64_t client_id);

  static inline std::flat_map<uint32_t, server_common_map_mob>
      common_map_mob; // 普通
  static inline std::flat_multimap<uint32_t, server_party_map_mob>
      party_map_mob; // 组队
};