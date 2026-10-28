#ifndef SDLMS_SRC_SERVER_SERVER_SERVER_SCENE_H_
#define SDLMS_SRC_SERVER_SERVER_SERVER_SCENE_H_

#include "src/server/server/server_client.h"
#include "src/server/server/server_drop.h"
#include "src/server/server/server_foothold.h"
#include "src/server/server/server_mob.h"
#include "src/server/server/server_reactor.h"
#include "src/server/server/server_region_skill.h"

#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <vector>

namespace sdlms {

struct ServerSceneState {
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

#endif  // SDLMS_SRC_SERVER_SERVER_SERVER_SCENE_H_
