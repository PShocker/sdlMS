#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/server.h"
#include "src/server/server/server_party.h"

#include <cstdint>
#include <flat_map>
#include <optional>

namespace sdlms {

class ServerPartyInstance {
 private:
  static ServerParty& LoadParty(uint64_t captain_id);

 public:
  static inline std::flat_map<uint64_t, ServerParty> data_;

  static std::optional<ServerParty> LoadClientParty(uint64_t client_id);

  static void HandleParty(uint64_t client_id, fbs::ClientCharacterPartyT& r);
  static void HandleServerParty(uint64_t client_id,
                                fbs::ServerCharacterPartyT& r);
  // 帧循环：清理过期邀请、成员掉线的队伍（095 PartyMember 有效性）。
  static bool Run();
  // 指定玩家在队伍中时向全员刷新一次 HP/状态（状态变化后调用）。
  static void SyncClientParty(uint64_t client_id);
};

}  // namespace sdlms
