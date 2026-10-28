#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/server.h"
using namespace fbs;

class ServerBallInstance {
public:
  static void HandleBall(uint64_t client_id, ClientCharacterBallT &r);
  static void HandleServerB(const std::unique_ptr<fbs::CharacterBallT> &r);
  static void HandleServerBall(uint64_t client_id, ServerCharacterBallT &r);
};