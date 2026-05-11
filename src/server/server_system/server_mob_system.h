#pragma once

#include "src/server/server/server_mob.h"
#include <cstdint>
class server_mob_system {
private:
  enum class action_enum {
    stand,
    jump,
    swim,
    fly,
    hit,
    remove, // 复活
  };
  static bool run_action(server_mob &s_mob, const std::u16string &action);
  static action_enum load_action_type(server_mob &s_mob);
  static void run_walk(server_mob &s_mob);
  static void run_state_machine(server_mob &s_mob);

public:
  static bool run();
};