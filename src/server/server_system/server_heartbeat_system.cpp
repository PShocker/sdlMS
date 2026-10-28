#include "server_heartbeat_system.h"
#include "SDL3/SDL_timer.h"
#include "src/client/system_instance/character_choose_system_instance.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/request/client_request.h"
#include "src/common/response/server_response.h"
#include "src/server/server_instance/server_client_instance.h"
#include "src/server/server_instance/server_heartbeat_instance.h"
#include "src/server/server_instance/server_scene_instance.h"
#include <cstdint>
#include <ranges>
#include <vector>
namespace sdlms {


bool ServerHeartbeatSystem::RunClientHeartbeat() {
  if (SceneSystemInstance::map_id_ == 0 ||
      SceneSystemInstance::prepare_map_id_ == 0) {
    return false;
  }
  auto current_time = Window::dt_now_;
  auto server_hb_time = ServerHeartbeatInstance::host_heartbeat;
  // 先检查有没有掉线
  if (server_hb_time != 0 &&
      (current_time - server_hb_time >=
       ServerHeartbeatInstance::heartbeat_interval * 2000)) {
    // 掉线了,客户端退出到人物选择界面
    CharacterChooseSystemInstance::Enter();
    return false;
  }
  // 客户端发送心跳
  static uint64_t last_execution_time = 0;
  // 检查是否到了执行时间
  if (current_time - last_execution_time >=
      ServerHeartbeatInstance::heartbeat_interval * 1000) {
    // 执行你的代码
    fbs::ClientHeartbeatT hb;
    ClientRequest::send_to_host(hb);
    // 更新最后执行时间
    last_execution_time = current_time;
  }
  return true;
}

bool ServerHeartbeatSystem::RunServerHeartbeat() {
  // 服务端主动检查心跳
  auto current_time = Window::dt_now_;
  auto &clients = ServerClientInstance::clients_;
  for (auto it = clients.begin(); it != clients.end();) {
    auto client = it->second;
    if (current_time - client.heartbeat >=
        ServerHeartbeatInstance::heartbeat_interval * 2000) {
      it = clients.erase(it);
      auto &scenes = ServerSceneInstance::scenes_[client.map_id];
      scenes.clients.erase(client.client_id);
      for (const auto other : scenes.clients) {
        fbs::ServerCharacterOutT r;
        r.client_id = client.client_id;
        ServerResponse::send_to_client(other, r);
      }
    } else {
      ++it;
    }
  }

  return true;
}

void ServerHeartbeatSystem::ReceiveClientHeartbeat(uint64_t client_id) {
  if (ServerClientInstance::clients_.contains(client_id)) {
    ServerClientInstance::clients_.at(client_id).heartbeat = Window::dt_now_;
  }
  fbs::ServerHeartbeatT r;
  ServerResponse::send_to_client(client_id, r);
}

void ServerHeartbeatSystem::ReceiveServerHeartbeat() {
  ServerHeartbeatInstance::host_heartbeat = Window::dt_now_;
}

}  // namespace sdlms
