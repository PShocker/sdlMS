#include "server_response.h"
#include "src/common/flatbuffers/Server.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/protocol.h"
#include "src/server/server_main.h"
#include <cstdint>

template <typename CreateFunc, typename... Args>
static void send_client(uint64_t client_id, NetPayload type, CreateFunc func,
                        Args &&...args) {
  auto payload = func(server_main::builder, std::forward<Args>(args)...);
  auto packet = CreateNetPacket(server_main::builder, type, payload.Union());
  server_main::builder.Finish(packet);
  auto addr = server_main::clients.at(client_id).addr;
  server_main::server_send(server_main::builder.GetBufferPointer(),
                           server_main::builder.GetSize(), addr);
  server_main::builder.Clear();
}

void server_response::server_heartbeat_response(uint64_t client_id) {
  send_client(client_id, NetPayload_ServerHeartbeat, CreateServerHeartbeat);
}

void server_response::server_move_response(uint64_t client_id, float x1,
                                           float x2, float y1, float y2,
                                           uint32_t time) {
  auto client = server_main::clients.at(client_id);
  auto scene = client.scene;
  auto scene_client = server_main::scene_clients.at(scene);
  for (auto c : scene_client) {
    if (c != client_id) {
      send_client(c, NetPayload_ServerMove, CreateServerMove, x1, x2, y1, y2,
                  time, client_id);
    }
  }
}