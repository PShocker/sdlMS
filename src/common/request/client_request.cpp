#include "client_request.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/protocol.h"
#include "src/server/server_main.h"

using namespace fbs;

template <typename T> static void send_to_host(T &message_data) {
  // 构建 NetPacket
  fbs::NetPacketT packet;
  fbs::NetPayloadUnion u;
  u.Set((message_data));
  packet.payload = u;

  // 序列化并发送
  flatbuffers::FlatBufferBuilder builder;
  auto packet_offset = fbs::NetPacket::Pack(builder, &packet);
  builder.Finish(packet_offset);
  auto addr = server_main::host ? &server_main::host_addr : nullptr;
  server_main::server_send(builder.GetBufferPointer(), builder.GetSize(), addr);
  builder.Clear();
}

void client_request::client_heartbeat_request() {
  fbs::ClientHeartbeatT hb;
  send_to_host(hb);
}

void client_request::client_scene_request(fbs::ClientSceneT client_scene) {
  send_to_host(client_scene);
}

void client_request::client_character_move_request(
    fbs::ClientCharacterMoveT client_character_move) {
  send_to_host(client_character_move);
}