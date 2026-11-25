#include "src/server/server_system_instance/server_system_instance.h"

#include "src/server/server_instance/server_minigame_instance.h"
#include "src/server/server_instance/server_party_instance.h"
#include "src/server/server_instance/server_player_shop_instance.h"
#include "src/server/server_main.h"
#include "src/server/server_system/server_drop_system.h"
#include "src/server/server_system/server_heartbeat_system.h"
#include "src/server/server_system/server_mob_system.h"
#include "src/server/server_system/server_reactor_system.h"
#include "src/server/server_system/server_system.h"

#include <algorithm>

namespace sdlms {

void ServerSystemInstance::Enter(bool host) {
  if (host) {
    ServerSystem::server_systems_ = {
        ServerMain::Run,
        ServerHeartbeatSystem::RunServerHeartbeat,
        ServerHeartbeatSystem::RunClientHeartbeat,
        ServerMobSystem::Run,
        ServerDropSystem::Run,
        ServerReactorSystem::Run,
        ServerMinigameInstance::Run,
        ServerPartyInstance::Run,
        ServerPlayerShopInstance::Run,
    };
  } else {
    ServerSystem::server_systems_ = {
        ServerMain::Run,
        ServerHeartbeatSystem::RunClientHeartbeat,
    };
  }
}

}  // namespace sdlms
