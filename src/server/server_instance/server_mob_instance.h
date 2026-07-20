#pragma once

#include "src/client/game/game_mob.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
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

  static void handle_s_attack(uint64_t client_id, AttackT &at);

  static void handle_attack(uint64_t client_id, ClientMobAttackT &r);
  static void handle_server_attack(ServerMobAttackT &r);

  static void handle_server_mv(const ServerMobMvT &m);
  static void handle_server_flip(const ServerMobFlipT &m);
  static void handle_server_action(const ServerMobActionT &m);
  static void handle_server_state(const ServerMobStateT &m);
  static void handle_server_die(const ServerMobDieT &m);
  static void handle_server_event(const ServerMobEventT &m);
};