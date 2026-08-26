#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/server.h"
#include <cstdint>

using namespace fbs;

class server_trap_instance {
public:
  static void handle_s_attack(uint64_t client_id, AttackT &at);
  static void handle_attack(uint64_t client_id, ClientTrapAttackT &r);
  static void handle_server_attack(uint64_t client_id, ServerTrapAttackT &r);
};