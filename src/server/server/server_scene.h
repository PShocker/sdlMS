#pragma once

#include "server_client.h"
#include "server_mob.h"
#include "server_reactor.h"
#include "src/server/server/server_drop.h"
#include "src/server/server/server_foothold.h"
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <vector>

class server_scene {
public:
  uint32_t map_id;

  std::flat_map<uint32_t, server_mob> mobs;
  std::flat_map<uint64_t, server_drop> drops;
  std::flat_set<uint64_t> clients;
  std::flat_map<int32_t, server_foothold> fhs;
  std::vector<server_reactor> reactors;

  bool unique = false;
};