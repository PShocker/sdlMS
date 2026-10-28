#ifndef SDLMS_SRC_SERVER_SERVER_SYSTEM_SERVER_REACTOR_SYSTEM_H_
#define SDLMS_SRC_SERVER_SERVER_SYSTEM_SERVER_REACTOR_SYSTEM_H_

#include "src/server/server/server_reactor.h"

namespace sdlms {

class ServerReactorSystem {
 private:
  static uint64_t LoadHitCd(ServerReactorState& r);

  static void RunScript(ServerReactorState& r);
  static void RunRevive(ServerReactorState& r);
  static void RunHit(ServerReactorState& r);
  static void RunDestroy();

 public:
  static inline uint32_t map_id_ = 0;

  static bool Run();
};

}  // namespace sdlms

#endif  // SDLMS_SRC_SERVER_SERVER_SYSTEM_SERVER_REACTOR_SYSTEM_H_
