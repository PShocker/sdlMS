#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/server.h"
using namespace fbs;

class ServerRegionSkillInstance {
public:
  static void HandleRskill(uint64_t client_id, ClientRSkillT &r);
  static void HandleSRskill(ServerRSkillT &r);
};