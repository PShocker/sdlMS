#pragma once

#include <cstdint>
class ServerHeartbeatSystem {
public:
  static bool RunClientHeartbeat();
  static bool RunServerHeartbeat(); // server only

  static void ReceiveClientHeartbeat(uint64_t client_id);
  static void ReceiveServerHeartbeat();
};