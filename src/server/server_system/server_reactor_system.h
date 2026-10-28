#pragma once

#include "src/server/server/server_reactor.h"
class ServerReactorSystem {
private:
  static uint64_t load_hit_cd(ServerReactorState &r);

  static void run_script(ServerReactorState &r);
  static void run_revive(ServerReactorState &r);
  static void run_hit(ServerReactorState &r);
  static void run_destroy();

public:
  static inline uint32_t map_id = 0;

  static bool run();
};