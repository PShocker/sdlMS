#include "client_request.h"
#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/protocol.h"
#include "src/server/server_main.h"

using namespace fbs;

template <typename T> static void send_to_host(T &message_data) {
  // 构建 NetPacket
  NetPacketT packet;
  NetPayloadUnion u;
  u.Set((message_data));
  packet.payload = u;

  // 序列化并发送
  flatbuffers::FlatBufferBuilder builder;
  auto packet_offset = NetPacket::Pack(builder, &packet);
  builder.Finish(packet_offset);
  auto addr = server_main::host ? &server_main::host_addr : nullptr;
  server_main::server_send(builder.GetBufferPointer(), builder.GetSize(), addr);
  builder.Clear();
}

void client_request::heartbeat_request() {
  ClientHeartbeatT hb;
  send_to_host(hb);
}

void client_request::scene_request(ClientSceneT client_scene) {
  send_to_host(client_scene);
}

void client_request::character_logic_request(ClientCharacterLogicT logic) {
  send_to_host(logic);
}

void client_request::character_attack_request(ClientCharacterAttackT atk) {
  send_to_host(atk);
}