#pragma once

#include "src/client/game/game_foothold.h"
#include "src/common/flatbuffers/server.h"
#include "src/server/server/server_mob.h"
#include <cstdint>
#include <string>
#include <vector>

using namespace fbs;

class ServerMobSystem {
private:
  static inline uint32_t map_id = 0;
  static inline uint32_t delta_time = 0;
  static inline std::flat_map<int32_t, GameFoothold> g_fhs;
  static inline ServerMobEventT events;

  static int load_mob_hit_cd(ServerMob &mob);

  static bool run_try_jump(ServerMob &mob);

  static bool run_hit_check(ServerMob &mob);

  static bool run_debuff_action(ServerMob &mob);

  static void run_hit_action(ServerMob &mob);
  static void run_default_action(ServerMob &mob);
  static void run_move_action(ServerMob &mob);
  static void run_stand_action(ServerMob &mob);
  static void run_fly_action(ServerMob &mob);
  static void run_swim_action(ServerMob &mob);
  static void run_die_action(ServerMob &mob);
  static void run_walk(ServerMob &mob);
  static void run_state_machine(ServerMob &mob);
  static void run_duration(ServerMob &mob);
  static bool run_hitting(ServerMob &mob);
  static void run_hit(ServerMob &mob);
  static void run_die(ServerMob &mob, uint64_t client_id);
  static void run_send();
  static bool run_fall(ServerMob &mob);

  static void run_network_sync_pos(ServerMob &mob, ServerMob &o_mob);
  static void run_network_sync_flip(ServerMob &mob, ServerMob &o_mob);
  static void run_network_sync_action(ServerMob &mob, ServerMob &o_mob);
  static void run_network_sync_hp(ServerMob &mob, ServerMob &o_mob);

  static void run_network_sync(ServerMob &mob, ServerMob &o_mob);

public:
  static bool run();
};