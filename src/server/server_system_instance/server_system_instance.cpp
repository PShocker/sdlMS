#include "server_system_instance.h"
#include "src/server/server_main.h"
#include "src/server/server_system/server_heartbeat_system.h"
#include "src/server/server_system/server_mob_system.h"
#include "src/server/server_system/server_system.h"
#include <algorithm>

void server_system_instance::enter(bool host) {
  if (host) {
    server_system::server_systems = {
        server_main::run,
        // server_heartbeat_system::run_server_heartbeat,
        // server_heartbeat_system::run_client_heartbeat,
        server_mob_system::run,
    };
  } else {
    server_system::server_systems = {
        server_main::run,
        // server_heartbeat_system::run_client_heartbeat,
    };
  }
}