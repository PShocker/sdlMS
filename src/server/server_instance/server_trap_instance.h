#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/server.h"
#include <cstdint>
namespace sdlms {



class ServerTrapInstance {
public:
  static void HandleSAttack(uint64_t client_id, fbs::AttackT &at);
  static void HandleAttack(uint64_t client_id, fbs::ClientTrapAttackT &r);
  static void HandleServerAttack(uint64_t client_id, fbs::ServerTrapAttackT &r);
};

}  // namespace sdlms
