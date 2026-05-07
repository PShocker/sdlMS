#include "request_handler.h"
#include "SDL3/SDL_timer.h"
#include "server_instance/server_scene_instance.h"
#include "server_system/server_heartbeat_system.h"
#include "server_system_instance/server_system_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/protocol.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"
#include "src/server/server_instance/server_character_instance.h"
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
    server_response::server_heartbeat_response(client_id);
    break;
  }
  case NetPayload_ClientScene: {
    auto payload = packet->payload_as_ClientScene();
    fbs::ClientSceneT r;
    payload->UnPackTo(&r);
    server_scene_instance::handle_scene(client_id, r);
    break;
  }
  case NetPayload_ClientCharacterMove: {
    auto payload = packet->payload_as_ClientCharacterMove();
    fbs::ClientCharacterMoveT r;
    payload->UnPackTo(&r);
    server_character_instance::handle_character_move(client_id, r);
    break;
  }
  case NetPayload_ServerHeartbeat: {
    server_heartbeat_system::receive_server_heartbeat();
    break;
  }
  case NetPayload_ServerScene: {
    auto payload = packet->payload_as_ServerScene();
    fbs::ServerSceneT r;
    payload->UnPackTo(&r);
    server_system_instance::create_client_heartbeat();
    scene_system_instance::enter(r.map_id);
    character_game_instance::load_others_character(r.players);
    break;
  }
  case NetPayload_ServerCharacterMove: {
    auto payload = packet->payload_as_ServerCharacterMove();
    fbs::ServerCharacterMoveT r;
    payload->UnPackTo(&r);
    // r.client_id
    break;
  }
  default:
    break;
  }
}