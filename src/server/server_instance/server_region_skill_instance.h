#pragma once

#include "src/common/flatbuffers/client.h"
using namespace fbs;

class server_region_skill_instance {
public:
  static void handle_r(uint64_t client_id, ClientRSkillT &r);
};