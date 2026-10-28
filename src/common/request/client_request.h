#ifndef SDLMS_SRC_COMMON_REQUEST_CLIENT_REQUEST_H_
#define SDLMS_SRC_COMMON_REQUEST_CLIENT_REQUEST_H_

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/protocol.h"
#include "src/server/server_main.h"
#include <cstdint>
namespace sdlms {



class ClientRequest {
public:
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
    auto addr = ServerMain::host_ ? &ServerMain::host_addr : nullptr;

    auto point = builder.GetBufferPointer();
    auto size = builder.GetSize();
    ServerMain::ServerSend(point, size, addr);
  }
};

}  // namespace sdlms

#endif  // SDLMS_SRC_COMMON_REQUEST_CLIENT_REQUEST_H_
