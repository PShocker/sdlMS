#pragma once

#include "src/server/server/server_reactor.h"
class server_reactor_system {
private:
  static void run_script(server_reactor &r);
  static void run_revive(server_reactor &r);
  static void run_destroy();

public:
  static inline uint32_t map_id = 0;

  static bool run();
};