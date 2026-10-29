#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/server.h"

namespace sdlms {

class ServerRegionSkillInstance {
 public:
  static void HandleRskill(uint64_t client_id, fbs::ClientRSkillT& r);
  static void HandleSRskill(fbs::ServerRSkillT& r);
};

}  // namespace sdlms
