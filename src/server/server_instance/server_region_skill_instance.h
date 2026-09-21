#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/server.h"
using namespace fbs;

class server_region_skill_instance {
public:
  static void handle_rskill(uint64_t client_id, ClientRSkillT &r);
  static void handle_s_rskill(ServerRSkillT &r);
};