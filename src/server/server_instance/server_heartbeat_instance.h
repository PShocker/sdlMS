#ifndef SDLMS_SRC_SERVER_SERVER_INSTANCE_SERVER_HEARTBEAT_INSTANCE_H_
#define SDLMS_SRC_SERVER_SERVER_INSTANCE_SERVER_HEARTBEAT_INSTANCE_H_

#include <cstdint>
namespace sdlms {

struct ServerHeartbeatInstance{
  static inline uint64_t host_heartbeat;
  const static inline uint8_t heartbeat_interval = 2;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_SERVER_SERVER_INSTANCE_SERVER_HEARTBEAT_INSTANCE_H_
