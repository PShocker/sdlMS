#include "server_system_instance.h"
#include "src/server/server_main.h"
#include "src/server/server_system/server_drop_system.h"
#include "src/server/server_system/server_heartbeat_system.h"
#include "src/server/server_system/server_mob_system.h"
#include "src/server/server_system/server_reactor_system.h"
#include "src/server/server_system/server_system.h"
#include <algorithm>

void ServerSystemInstance::Enter(bool host) {
  if (host) {
    ServerSystem::server_systems = {
        ServerMain::Run,
        // server_heartbeat_system::run_server_heartbeat,
        // server_heartbeat_system::run_client_heartbeat,
        ServerMobSystem::Run,
        ServerDropSystem::Run,
        ServerReactorSystem::Run,
    };
  } else {
    ServerSystem::server_systems = {
        ServerMain::Run,
        // server_heartbeat_system::run_client_heartbeat,
    };
  }
}