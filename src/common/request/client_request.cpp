#include "client_request.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/protocol.h"
#include "src/server/server_main.h"

template <typename CreateFunc, typename... Args>
static void send_host(NetPayload type, CreateFunc func, Args &&...args) {
  auto payload = func(server_main::builder, std::forward<Args>(args)...);
  auto packet = CreateNetPacket(server_main::builder, type, payload.Union());
  server_main::builder.Finish(packet);
  server_main::server_send(server_main::builder.GetBufferPointer(),
                           server_main::builder.GetSize(),
                           server_main::host_addr);
  server_main::builder.Clear();
}

void client_request::client_scene_request(bool come, uint32_t map_id) {
  send_host(NetPayload_ClientScene, CreateClientScene, come, map_id);
}

void client_request::client_heartbeat_request() {
  send_host(NetPayload_ClientHeartbeat, CreateClientHeartbeat);
}

void client_request::client_move_request(float x1, float y1, float x2, float y2,
                                         uint32_t time) {
  send_host(NetPayload_ClientMove, CreateClientMove, x1, y1, x2, y2, time);
}