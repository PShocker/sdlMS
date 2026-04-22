#pragma once

#include "src/server/server/server_mob.h"
#include <cstdint>
#include <flat_map>
#include <vector>
class server_mob_instance {
public:
  static std::flat_map<uint32_t, server_mob> load_mob(uint32_t map_id);
  static void load_summon_mob(uint64_t client_id);

  static void load_vsb_mob(uint32_t m_id, std::vector<uint32_t> &mb_ids);
};