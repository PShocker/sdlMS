#pragma once

#include "src/client/game/game_mob.h"
#include "src/common/flatbuffers/client.h"
#include "src/server/server/server_mob.h"
#include "src/server/server/server_scene.h"
#include <cstdint>
#include <flat_map>
#include <vector>

using namespace fbs;

class server_mob_instance {
public:
  static void load_mob(server_scene &scene);
  static void load_summon_mob(uint64_t client_id);

  static void handle_attack(uint64_t client_id, ClientMobAttackT &r);
};