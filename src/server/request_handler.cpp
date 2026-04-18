#include "request_handler.h"
#include "SDL3/SDL_timer.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/common/flatbuffers/protocol.h"
#include "src/common/response/server_response.h"
#include "src/server/server_main.h"
#include <cstdio>
#include <ctime>

using namespace fbs;

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
  case NetPayload_ClientScene: {
    auto payload = packet->payload_as_ClientScene();
    fbs::ServerSceneT r;
    r.scene_host = client_id;
    r.scene_id = payload->scene_id();
    r.your_id = client_id;
    server_response::server_scene_response(client_id, r);
    break;
  }

  case NetPayload_ServerHeartbeat: {
    server_main::host_heartbeat = SDL_GetTicks();
    break;
  }
  case NetPayload_ServerScene: {
    auto payload = packet->payload_as_ServerScene();
    auto scene_id = payload->scene_id();
    scene_system_instance::enter(scene_id);
    break;
  }

  default:
    break;
  }
}