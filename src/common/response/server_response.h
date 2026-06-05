#pragma once

#include "src/common/flatbuffers/protocol.h"
#include "src/common/flatbuffers/server.h"
#include "src/server/server_main.h"
#include <cstdint>

using namespace fbs;

class server_response {
public:
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
  }
};