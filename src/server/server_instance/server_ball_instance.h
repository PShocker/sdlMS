#ifndef SDLMS_SRC_SERVER_SERVER_INSTANCE_SERVER_BALL_INSTANCE_H_
#define SDLMS_SRC_SERVER_SERVER_INSTANCE_SERVER_BALL_INSTANCE_H_

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/server.h"
namespace sdlms {


class ServerBallInstance {
 public:
  static void HandleBall(uint64_t client_id, fbs::ClientCharacterBallT& r);
  static void HandleServerB(const std::unique_ptr<fbs::CharacterBallT>& r);
  static void HandleServerBall(uint64_t client_id,
                               fbs::ServerCharacterBallT& r);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_SERVER_SERVER_INSTANCE_SERVER_BALL_INSTANCE_H_
