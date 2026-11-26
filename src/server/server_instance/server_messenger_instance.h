#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/server.h"

#include <cstdint>

namespace sdlms {

// MapleChat 聊天室（095 CUIMessenger，CP/SP 143）：
// 服务端维护房间/邀请/转发，HandleServer 写客户端 UI 状态。
class ServerMessengerInstance {
 public:
  static void HandleMessenger(uint64_t client_id,
                              fbs::ClientCharacterMessengerT& r);
  // 客户端收到 ServerCharacterMessenger 的处理（写 MapleChatUiSystem）。
  static void HandleServer(uint64_t client_id,
                           fbs::ServerCharacterMessengerT& r);
  // 帧循环：掉线成员离房广播、过期邀请清理、空房删除。
  static bool Run();
};

}  // namespace sdlms
