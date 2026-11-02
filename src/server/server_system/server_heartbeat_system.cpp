#include "src/server/server_system/server_heartbeat_system.h"

#include "src/client/system_instance/fade_system_instance.h"
#include "src/client/system_instance/login_notice_system_instance.h"
#include "src/client/system_instance/login_system_instance.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/request/client_request.h"
#include "src/common/response/server_response.h"
#include "src/server/server_instance/server_client_instance.h"
#include "src/server/server_instance/server_heartbeat_instance.h"
#include "src/server/server_instance/server_scene_instance.h"
#include "src/server/server_main.h"

#include <SDL3/SDL_timer.h>

#include <cstdint>
#include <ranges>
#include <vector>

namespace sdlms {

bool ServerHeartbeatSystem::RunClientHeartbeat() {
  // 主机就是服务端本身：不受心跳超时限制，也不发送心跳包。
  if (ServerMain::host_) {
    return true;
  }
  if (!ServerHeartbeatInstance::client_started) {
    return true;
  }
  auto current_time = Window::dt_now_;
  auto server_hb_time = ServerHeartbeatInstance::host_heartbeat;
  // 先检查有没有掉线
  if (server_hb_time != 0 &&
      (current_time - server_hb_time >=
       ServerHeartbeatInstance::heartbeat_timeout_ms)) {
    // 心跳断开：先渐隐到黑屏，再回到登录页并显示无法连接提示。
    ServerHeartbeatInstance::client_started = false;
    FadeSystemInstance::EnterIn([] {
      LoginSystemInstance::Enter();
      LoginNoticeSystemInstance::Enter(
          LoginNoticeSystemInstance::NoticeType::kUnableToConnectToGameSvr,
          nullptr);
      // 与 SceneSystemInstance::EnterQuit 的地图切换渐变保持一致。
      FadeSystemInstance::EnterOut();
      FadeSystemInstance::EnterOut();
    });
    return false;
  }
  // 客户端发送心跳
  static uint64_t last_execution_time = 0;
  // 检查是否到了执行时间
  if (current_time - last_execution_time >=
      ServerHeartbeatInstance::heartbeat_interval_ms) {
    fbs::ClientHeartbeatT hb;
    ClientRequest::SendToHost(hb);
    // 更新最后执行时间
    last_execution_time = current_time;
  }
  return true;
}

bool ServerHeartbeatSystem::RunServerHeartbeat() {
  // 服务端主动检查心跳
  auto current_time = Window::dt_now_;
  auto& clients = ServerClientInstance::clients_;
  for (auto it = clients.begin(); it != clients.end();) {
    // 主机客户端不参与服务端心跳超时检查。
    if (it->first == ServerMain::local_addr_) {
      ++it;
      continue;
    }
    auto client = it->second;
    if (current_time - client.heartbeat >=
        ServerHeartbeatInstance::heartbeat_timeout_ms) {
      it = clients.erase(it);
      auto& scenes = ServerSceneInstance::scenes_[client.map_id];
      scenes.clients.erase(client.client_id);
      for (const auto other : scenes.clients) {
        fbs::ServerCharacterOutT r;
        r.client_id = client.client_id;
        ServerResponse::SendToClient(other, r);
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
  ServerResponse::SendToClient(client_id, r);
}

void ServerHeartbeatSystem::ReceiveServerHeartbeat() {
  ServerHeartbeatInstance::host_heartbeat = Window::dt_now_;
}

}  // namespace sdlms
