#include "server_response.h"
#include "src/common/flatbuffers/protocol.h"
#include "src/common/flatbuffers/server.h"
#include "src/server/server_main.h"
#include <cstdint>

using namespace fbs;

template <typename T>
static void send_to_client(uint64_t client_id, T &message_data) {
  // 构建 NetPacket
  NetPacketT packet;
  NetPayloadUnion u;
  u.Set((message_data));
  packet.payload = u;

  // 序列化并发送
  flatbuffers::FlatBufferBuilder builder;
  auto packet_offset = NetPacket::Pack(builder, &packet);
  builder.Finish(packet_offset);
  server_main::server_send(builder.GetBufferPointer(), builder.GetSize(),
                           client_id);
  builder.Clear();
}

void server_response::heartbeat_response(uint64_t client_id) {
  ServerHeartbeatT r;
  send_to_client(client_id, r);
}

void server_response::scene_response(uint64_t client_id, ServerSceneT r) {
  send_to_client(client_id, r);
}

void server_response::character_in_response(uint64_t client_id,
                                            ServerCharacterInT r) {
  send_to_client(client_id, r);
}

void server_response::character_out_response(uint64_t client_id,
                                             ServerCharacterOutT r) {
  send_to_client(client_id, r);
}

void server_response::character_logic_response(uint64_t client_id,
                                               ServerCharacterLogicT r) {
  send_to_client(client_id, r);
}

void server_response::mob_logic_response(uint64_t client_id,
                                         ServerMobLogicT r) {
  send_to_client(client_id, r);
}

void server_response::character_attack_response(uint64_t client_id,
                                                ServerCharacterAttackT r) {
  send_to_client(client_id, r);
}