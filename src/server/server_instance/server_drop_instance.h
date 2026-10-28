#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/server/server/server_drop.h"
#include <cstdint>
#include <flat_map>
namespace sdlms {


using namespace fbs;

class ServerDropInstance {
private:
  static float CalAvailableTime(float y1, float y2);

public:
  static void SaveDrop(uint64_t map_id, const DropT &drop);
  static void SaveDelayDrop(uint64_t map_id, uint64_t d, const DropT &drop);

  static std::vector<std::unique_ptr<fbs::DropT>>
  CreateDts(const std::vector<DropT> &dts, uint32_t map_id);

  static void HandleServerDt(const DropT &dt);
  static void HandleServerSceneDt(const DropT &dt);
  static void HandleClientDrop(uint64_t client_id, ClientCharacterDropT &r);
  static void HandleClientDropFade(ClientDropFadeT &r);
  static void HandlePick(uint64_t client_id, ClientCharacterPickT &r);
  static void HandleServerPick(uint64_t client_id, ServerCharacterPickT &r);
  static void HandleServerDrop(uint64_t client_id, ServerCharacterDropT &r);

  static void HandleServerDropFade(ServerDropFadeT &r);

  static void HandleClientCreateDrop(uint64_t client_id,
                                        ClientCreateDropT &r);
  static void HandleServerCreateDrop(uint64_t client_id,
                                        ServerCreateDropT &r);
};

}  // namespace sdlms
