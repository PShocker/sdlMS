#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/server.h"
#include <cstdint>
namespace sdlms {


using namespace fbs;

class ServerTrapInstance {
public:
  static void HandleSAttack(uint64_t client_id, AttackT &at);
  static void HandleAttack(uint64_t client_id, ClientTrapAttackT &r);
  static void HandleServerAttack(uint64_t client_id, ServerTrapAttackT &r);
};

}  // namespace sdlms
