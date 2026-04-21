#include "server_system_instance.h"
#include "src/server/server_system/server_heartbeat_system.h"
#include "src/server/server_system/server_mob_system.h"
#include "src/server/server_system/server_system.h"
#include <algorithm>

void server_system_instance::enter(bool host) {
  if (host) {
    server_system::server_systems = {
        server_heartbeat_system::server_heartbeat,
        server_mob_system::run,
    };
  }
}

void server_system_instance::create_client_heartbeat() {
  auto fn = &server_heartbeat_system::client_heartbeat;
  if (!std::ranges::contains(server_system::server_systems, fn)) {
    server_system::server_systems.push_back(fn);
  }
}