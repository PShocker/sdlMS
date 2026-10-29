#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/protocol.h"
#include "src/server/server_main.h"

#include <cstdint>

namespace sdlms {

class ClientRequest {
 public:
  template <typename T>
  static void SendToHost(T& message_data) {
    // 构建 NetPacket
    fbs::NetPacketT packet;
    fbs::NetPayloadUnion u;
    u.Set((message_data));
    packet.payload = u;

    // 序列化并发送
    flatbuffers::FlatBufferBuilder builder;
    auto packet_offset = fbs::NetPacket::Pack(builder, &packet);
    builder.Finish(packet_offset);
    auto addr = ServerMain::host_ ? &ServerMain::host_addr_ : nullptr;

    auto point = builder.GetBufferPointer();
    auto size = builder.GetSize();
    ServerMain::ServerSend(point, size, addr);
  }
};

}  // namespace sdlms
