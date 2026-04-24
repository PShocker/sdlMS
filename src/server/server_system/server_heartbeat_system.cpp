#include "server_heartbeat_system.h"
#include "SDL3/SDL_timer.h"
#include "src/client/system_instance/character_choose_system_instance.h"
#include "src/server/server_instance/server_client_instance.h"
#include "src/server/server_instance/server_heartbeat_instance.h"
#include <cstdint>
#include <ranges>
#include <vector>

bool server_heartbeat_system::client_heartbeat() {
  auto current_time = SDL_GetTicks();
  auto server_hb_time = server_heartbeat_instance::host_heartbeat;
  // 先检查有没有掉线
  if (current_time - server_hb_time >=
      server_heartbeat_instance::heartbeat_interval * 2000) {
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

    // 更新最后执行时间
    last_execution_time = current_time;
  }
  return true;
}

bool server_heartbeat_system::server_heartbeat() {
  // 服务端主动检查心跳
  auto current_time = SDL_GetTicks();
  std::vector<uint64_t> exit_clients;
  for (auto &c : server_client_instance::clients | std::views::values) {
    if (current_time - c.heartbeat >=
        server_heartbeat_instance::heartbeat_interval * 2000) {
      // 掉线了,客户端退出到人物选择界面
      exit_clients.push_back(c.id);
    }
  }
  for (auto c : exit_clients) {
  
  }
  return true;
}

void server_heartbeat_system::receive_server_heartbeat() {
  server_heartbeat_instance::host_heartbeat = SDL_GetTicks();
}