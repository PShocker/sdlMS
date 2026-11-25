#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/server.h"

#include <cstdint>

namespace sdlms {

// 五子棋房间（095 CMiniRoomBaseDlg / COmokDlg）：
// 一张地图一间房，房主 = slot0 执黑先行，客方 = slot1 执白。
class ServerMinigameInstance {
 public:
  // 房主侧：处理玩家的房间操作（仅主机执行）。
  static void HandleClient(uint64_t client_id,
                           fbs::ClientCharacterMinigameT& r);
  // 客户端侧：应用服务端下发的房间状态同步。
  static void HandleServer(uint64_t client_id,
                           fbs::ServerCharacterMinigameT& r);
  // 主机帧循环：对局中 30 秒无落子判负（095 m_nTimeLeft）。
  static bool Run();
};

}  // namespace sdlms