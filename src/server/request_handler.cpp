#include "request_handler.h"
#include "SDL3/SDL_timer.h"
#include "src/common/flatbuffers/protocol.h"
#include "src/server/server_main.h"
#include <cstdio>
#include <ctime>

void request_handler::handle_request(uint64_t client_id, void *buf,
                                     ssize_t nread) {
  auto packet = GetNetPacket(buf);
  if (!packet) {
    printf("error handle_request\n");
    return;
  }
  switch (packet->payload_type()) {
  case NetPayload_ClientHeartbeat: {
    break;
  }
  case NetPayload_ClientInScene: {
    auto payload = packet->payload_as_ClientInScene();
    break;
  }
  case NetPayload_ClientOutScene: {
    break;
  }
  case NetPayload_ClientMove: {
    break;
  }
  case NetPayload_ServerHeartbeat: {
    server_main::host_heartbeat = SDL_GetTicks();
    break;
  }
  case NetPayload_ServerInScene: {
    break;
  }
  case NetPayload_ServerOutScene: {
    break;
  }
  case NetPayload_ServerMove: {
    break;
  }

  default:
    break;
  }
}