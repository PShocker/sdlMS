#pragma once

#include "src/server/server/server_mob.h"
#include "src/server/server/server_scene.h"
#include <cstdint>
#include <flat_map>
#include <vector>
class server_mob_instance {
public:
  static void load_mob(server_scene &s_scene);
  static void load_summon_mob(uint64_t client_id);
};