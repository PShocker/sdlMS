#pragma once

#include <cstdint>
class ServerHeartbeatSystem {
public:
  static bool run_client_heartbeat();
  static bool run_server_heartbeat(); // server only

  static void receive_client_heartbeat(uint64_t client_id);
  static void receive_server_heartbeat();
};