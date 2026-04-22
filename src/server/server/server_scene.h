#pragma once

#include "server_client.h"
#include "server_mob.h"
#include "src/server/server/server_drop.h"
#include "src/server/server/server_foothold.h"
#include <cstdint>
#include <flat_map>

class server_scene {
public:
  uint32_t map_id;

  std::flat_map<uint32_t, server_mob> mobs;
  std::flat_map<uint32_t, server_drop> drops;
  std::flat_map<uint64_t, server_client> clients;
  std::flat_map<int32_t, server_foothold> fhs;

  bool unique = false;
};