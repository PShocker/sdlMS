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

bool server_heartbeat_system::run_client_heartbeat() {
  if (scene_system_instance::map_id == 0 ||
      scene_system_instance::prepare_map_id == 0) {
    return false;
  }
  auto current_time = window::dt_now;
  auto server_hb_time = server_heartbeat_instance::host_heartbeat;
  // 先检查有没有掉线
  if (server_hb_time != 0 &&
      (current_time - server_hb_time >=
       server_heartbeat_instance::heartbeat_interval * 2000)) {
    // 掉线了,客户端退出到人物选择界面
    character_choose_system_instance::enter();
    return false;
  }
  // 客户端发送心跳
  static uint64_t last_execution_time = 0;
  // 检查是否到了执行时间
  if (current_time - last_execution_time >=
      server_heartbeat_instance::heartbeat_interval * 1000) {
    // 执行你的代码
    client_request::client_heartbeat_request();
    // 更新最后执行时间
    last_execution_time = current_time;
  }
  return true;
}

bool server_heartbeat_system::run_server_heartbeat() {
  // 服务端主动检查心跳
  auto current_time = window::dt_now;
  auto &clients = server_client_instance::clients;
  for (auto it = clients.begin(); it != clients.end();) {
    auto client = it->second;
    if (current_time - client.heartbeat >=
        server_heartbeat_instance::heartbeat_interval * 2000) {
      it = clients.erase(it);
      auto &scenes = server_scene_instance::scenes[client.map_id];
      scenes.clients.erase(client.client_id);
      for (const auto other : scenes.clients) {
        fbs::ServerCharacterOutT r;
        r.client_id = client.client_id;
        server_response::server_character_out_response(other, r);
      }
    } else {
      ++it;
    }
  }

  return true;
}

void server_heartbeat_system::receive_client_heartbeat(uint64_t client_id) {
  if (server_client_instance::clients.contains(client_id)) {
    server_client_instance::clients.at(client_id).heartbeat = window::dt_now;
  }
  server_response::server_heartbeat_response(client_id);
}

void server_heartbeat_system::receive_server_heartbeat() {
  server_heartbeat_instance::host_heartbeat = window::dt_now;
}