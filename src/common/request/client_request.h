#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/protocol.h"
#include "src/server/server_main.h"
#include <cstdint>

using namespace fbs;

class client_request {
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
    auto addr = server_main::host ? &server_main::host_addr : nullptr;
    server_main::server_send(builder.GetBufferPointer(), builder.GetSize(),
                             addr);
  }
};