#pragma once

#include "src/common/flatbuffers/server.h"
#include "src/server/server/server_mob.h"
#include <cstdint>
#include <string>
#include <vector>

using namespace fbs;

class server_mob_system {
private:
  static inline uint32_t map_id = 0;
  static inline uint32_t delta_time = 0;
  static inline ServerMobEventT events;

  struct mob_drop {
    std::u16string id;
    int min_quantity;
    int max_quantity;
    float rate;
  };
  static std::vector<mob_drop> load_mob_drops(server_mob &mob);

  static int load_mob_hit_cd(server_mob &mob);

  static bool run_try_jump(server_mob &mob);

  static bool run_hit_check(server_mob &mob);

  static void run_hit_action(server_mob &mob);
  static void run_move_action(server_mob &mob);
  static void run_stand_action(server_mob &mob);
  static void run_die_action(server_mob &mob);
  static void run_walk(server_mob &mob);
  static void run_state_machine(server_mob &mob);
  static void run_duration(server_mob &mob);
  static bool run_hitting(server_mob &mob);
  static void run_hit(server_mob &mob);
  static void run_die(server_mob &mob, uint64_t client_id);
  static void run_send();

  static void run_network_sync(server_mob &mob, server_mob &o_mob);

public:
  static bool run();
};