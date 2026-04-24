#pragma once

#include "src/server/server/server_foothold.h"
#include "src/server/server/server_scene.h"
#include <flat_map>

class server_foothold_instance {
public:
  static void load_fh(server_scene &s_scene);
};