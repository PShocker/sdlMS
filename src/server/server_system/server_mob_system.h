#pragma once

#include "src/common/flatbuffers/server.h"
#include "src/server/server/server_mob.h"
#include <cstdint>
#include <vector>

using namespace fbs;

class server_mob_system {
private:
  static inline uint32_t map_id = 0;
  static inline uint32_t delta_time = 0;
  static inline std::vector<std::unique_ptr<MobLogicT>> unique_logics;

  static void run_hit_action(server_mob &s_mob);
  static void run_move_action(server_mob &s_mob);
  static void run_stand_action(server_mob &s_mob);
  static void run_die_action(server_mob &s_mob);
  static void run_walk(server_mob &s_mob);
  static void run_state_machine(server_mob &s_mob);
  static void run_duration(server_mob &s_mob);
  static bool run_beat(server_mob &s_mob);
  static void run_hit(server_mob &s_mob);
  static void run_send();

  static void run_network_action_sync(server_mob &s_mob, server_mob &o_mob);
  static void run_network_flip_sync(server_mob &s_mob, server_mob &o_mob);
  static void run_network_movement_sync(server_mob &s_mob, server_mob &o_mob);
  static void run_network_sync(server_mob &s_mob, server_mob &o_mob);

public:
  static bool run();
};