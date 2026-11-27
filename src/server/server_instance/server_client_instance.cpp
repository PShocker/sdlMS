#include "src/server/server_instance/server_client_instance.h"

#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"
#include "src/server/server_instance/server_character_instance.h"
#include "src/server/server_instance/server_scene_instance.h"

#include <ranges>
#include <string>

namespace sdlms {

// 好友上线通知：服务端在角色首次进图时广播给所有客户端，
// 客户端按自己的好友列表（人物唯一 id）过滤后弹 poptip。
// 按需求只做上线通知，不做下线通知。
void ServerClientInstance::BroadcastFriendOnline(uint64_t character_id,
                                                 const std::u16string& name) {
  if (character_id == 0) {
    return;
  }
  fbs::ServerFriendOnlineT r;
  r.id = character_id;
  r.name = std::vector<uint16_t>(name.begin(), name.end());
  r.online = true;
  for (auto [other, client] : clients_) {
    // 只发给已经建立角色数据的连接。
    if (client.player_t.character == nullptr) {
      continue;
    }
    ServerResponse::SendToClient(other, r);
  }
}

void ServerClientInstance::ClientExit(uint64_t client_id) {
  if (clients_.contains(client_id)) {
    auto map_id = clients_[client_id].map_id;
    auto& map_clients = ServerSceneInstance::scenes_.at(map_id).clients;
    map_clients.erase(map_id);
    ServerClientInstance::clients_.erase(client_id);
  }
  // 断线的玩家不再保留宠物状态。
  ServerCharacterInstance::pets_.erase(client_id);
  return;
}

}  // namespace sdlms