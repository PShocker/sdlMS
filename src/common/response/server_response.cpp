#include "server_response.h"
#include "src/common/flatbuffers/protocol.h"
#include "src/common/flatbuffers/server.h"
#include "src/server/server_main.h"
#include <cstdint>

using namespace fbs;

template <typename T>
static void send_to_client(uint64_t client_id, T &message_data) {
  // 构建 NetPacket
  fbs::NetPacketT packet;
  fbs::NetPayloadUnion u;
  u.Set((message_data));
  packet.payload = u;

  // 序列化并发送
  flatbuffers::FlatBufferBuilder builder;
  auto packet_offset = fbs::NetPacket::Pack(builder, &packet);
  builder.Finish(packet_offset);
  server_main::server_send(builder.GetBufferPointer(), builder.GetSize(),
                           client_id);
  builder.Clear();
}

void server_response::server_heartbeat_response(uint64_t client_id) {
  fbs::ServerHeartbeatT r;
  send_to_client(client_id, r);
}

void server_response::server_scene_response(uint64_t client_id,
                                            fbs::ServerSceneT r) {
  send_to_client(client_id, r);
}

void server_response::server_character_in_response(uint64_t client_id,
                                                   fbs::ServerCharacterInT r) {
  send_to_client(client_id, r);
}

void server_response::server_character_move_response(
    uint64_t client_id, fbs::ServerCharacterMoveT r) {
  send_to_client(client_id, r);
}

void server_response::server_mob_move_response(
    uint64_t client_id, fbs::ServerMobMoveT server_move) {}