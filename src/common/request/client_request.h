#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/protocol.h"
#include "src/server/server_main.h"
#include <cstdint>

using namespace fbs;

class ClientRequest {
public:
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
    auto addr = ServerMain::host_ ? &ServerMain::host_addr : nullptr;

    auto point = builder.GetBufferPointer();
    auto size = builder.GetSize();
    ServerMain::ServerSend(point, size, addr);
  }
};