#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/server.h"
using namespace fbs;

class server_ball_instance {
public:
  static void handle_ball(uint64_t client_id, ClientCharacterBallT &r);
  static void handle_server_b(const std::unique_ptr<fbs::CharacterBallT> &r);
  static void handle_server_ball(uint64_t client_id, ServerCharacterBallT &r);
};