#ifndef SDLMS_SRC_SERVER_SERVER_SYSTEM_SERVER_HEARTBEAT_SYSTEM_H_
#define SDLMS_SRC_SERVER_SERVER_SYSTEM_SERVER_HEARTBEAT_SYSTEM_H_

#include <cstdint>

namespace sdlms {

class ServerHeartbeatSystem {
 public:
  static bool RunClientHeartbeat();
  static bool RunServerHeartbeat();  // server only

  static void ReceiveClientHeartbeat(uint64_t client_id);
  static void ReceiveServerHeartbeat();
};

}  // namespace sdlms

#endif  // SDLMS_SRC_SERVER_SERVER_SYSTEM_SERVER_HEARTBEAT_SYSTEM_H_
