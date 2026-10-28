#ifndef SDLMS_SRC_COMMON_RESPONSE_SERVER_RESPONSE_H_
#define SDLMS_SRC_COMMON_RESPONSE_SERVER_RESPONSE_H_

#include "src/common/flatbuffers/protocol.h"
#include "src/common/flatbuffers/server.h"
#include "src/server/server_main.h"

#include <cstdint>

namespace sdlms {

class ServerResponse {
 public:
  template <typename T>
  static void send_to_client(uint64_t client_id, T& message_data) {
    // 构建 NetPacket
    fbs::NetPacketT packet;
    fbs::NetPayloadUnion u;
    u.Set((message_data));
    packet.payload = u;

    // 序列化并发送
    flatbuffers::FlatBufferBuilder builder;
    auto packet_offset = fbs::NetPacket::Pack(builder, &packet);
    builder.Finish(packet_offset);

    auto point = builder.GetBufferPointer();
    auto size = builder.GetSize();
    ServerMain::ServerSend(point, size, client_id);
  }
};

}  // namespace sdlms

#endif  // SDLMS_SRC_COMMON_RESPONSE_SERVER_RESPONSE_H_
