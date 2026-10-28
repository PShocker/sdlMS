#ifndef SDLMS_SRC_SERVER_SERVER_INSTANCE_SERVER_DROP_INSTANCE_H_
#define SDLMS_SRC_SERVER_SERVER_INSTANCE_SERVER_DROP_INSTANCE_H_

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/server/server/server_drop.h"

#include <cstdint>
#include <flat_map>

namespace sdlms {

class ServerDropInstance {
 private:
  static float CalAvailableTime(float y1, float y2);

 public:
  static void SaveDrop(uint64_t map_id, const fbs::DropT& drop);
  static void SaveDelayDrop(uint64_t map_id, uint64_t d,
                            const fbs::DropT& drop);

  static std::vector<std::unique_ptr<fbs::DropT>> CreateDts(
      const std::vector<fbs::DropT>& dts, uint32_t map_id);

  static void HandleServerDt(const fbs::DropT& dt);
  static void HandleServerSceneDt(const fbs::DropT& dt);
  static void HandleClientDrop(uint64_t client_id,
                               fbs::ClientCharacterDropT& r);
  static void HandleClientDropFade(fbs::ClientDropFadeT& r);
  static void HandlePick(uint64_t client_id, fbs::ClientCharacterPickT& r);
  static void HandleServerPick(uint64_t client_id,
                               fbs::ServerCharacterPickT& r);
  static void HandleServerDrop(uint64_t client_id,
                               fbs::ServerCharacterDropT& r);

  static void HandleServerDropFade(fbs::ServerDropFadeT& r);

  static void HandleClientCreateDrop(uint64_t client_id,
                                     fbs::ClientCreateDropT& r);
  static void HandleServerCreateDrop(uint64_t client_id,
                                     fbs::ServerCreateDropT& r);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_SERVER_SERVER_INSTANCE_SERVER_DROP_INSTANCE_H_
