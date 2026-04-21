#include "server_heartbeat_system.h"
#include "SDL3/SDL_timer.h"
#include "src/server/server_instance/server_heartbeat_instance.h"

bool server_heartbeat_system::client_heartbeat() { return true; }

bool server_heartbeat_system::server_heartbeat() { return true; }

void server_heartbeat_system::receive_server_heartbeat() {
  server_heartbeat_instance::host_heartbeat = SDL_GetTicks();
}