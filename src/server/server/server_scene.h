#pragma once

#include "server_client.h"
#include "server_mob.h"
#include "server_reactor.h"
#include "server_region_skill.h"
#include "src/server/server/server_drop.h"
#include "src/server/server/server_foothold.h"
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <vector>
namespace sdlms {


struct ServerSceneState{
  uint32_t map_id;

  std::flat_map<uint32_t, ServerMob> mobs;
  std::flat_map<uint64_t, ServerDrop> drops;
  std::flat_map<uint64_t, std::vector<ServerDrop>> delay_drops;
  std::flat_set<uint64_t> clients;
  std::flat_map<int32_t, ServerFoothold> fhs;
  std::vector<ServerReactorState> reactors;
  std::vector<ServerRegionSkill> rskis;

  bool unique = false;
};

}  // namespace sdlms
