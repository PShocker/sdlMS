#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/server/server/server_reactor.h"
#include "src/server/server/server_scene.h"
#include <cstdint>
#include <optional>
#include <string>
namespace sdlms {

class ServerReactorInstance {
public:
  static void LoadReactor(ServerSceneState &scene);
  static uint64_t LoadHitCd(ServerReactorState &r);
  static int LoadNextState(const std::u16string &id, uint8_t state);
  static std::u16string LoadScript(const std::u16string &id);

  static void HandleSReactor(const fbs::ReactorT &r);

  static void HandleReactor(uint64_t client_id, fbs::ClientReactorT &r);
  static void HandleServerReactor(uint64_t client_id, fbs::ServerReactorT &r);
  static void HandleServerReactorDrop(fbs::ServerReactorDropT &r);
};

}  // namespace sdlms
