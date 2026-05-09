#pragma once

#include "server_client.h"
#include "server_mob.h"
#include "src/server/server/server_drop.h"
#include "src/server/server/server_foothold.h"
#include <cstdint>
#include <flat_map>
#include <flat_set>

class server_scene {
public:
  uint32_t map_id;

  std::flat_map<uint32_t, server_mob> mobs;
  std::flat_map<uint32_t, server_drop> drops;
  std::flat_set<uint64_t> clients;
  std::flat_map<int32_t, server_foothold> fhs;

  bool unique = false;
};