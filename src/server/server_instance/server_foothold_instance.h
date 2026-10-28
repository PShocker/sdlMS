#ifndef SDLMS_SRC_SERVER_SERVER_INSTANCE_SERVER_FOOTHOLD_INSTANCE_H_
#define SDLMS_SRC_SERVER_SERVER_INSTANCE_SERVER_FOOTHOLD_INSTANCE_H_

#include "src/server/server/server_foothold.h"
#include "src/server/server/server_scene.h"

#include <flat_map>
namespace sdlms {


class ServerFootholdInstance {
 public:
  static void LoadFh(ServerSceneState& scene);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_SERVER_SERVER_INSTANCE_SERVER_FOOTHOLD_INSTANCE_H_
