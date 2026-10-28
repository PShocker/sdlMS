#pragma once

#include "src/server/server/server_foothold.h"
#include "src/server/server/server_scene.h"
#include <flat_map>

class ServerFootholdInstance {
public:
  static void LoadFh(ServerSceneState &scene);
};