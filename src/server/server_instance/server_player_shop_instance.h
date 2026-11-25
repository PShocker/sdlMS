#pragma once

#include "src/common/flatbuffers/client.h"
#include "src/common/flatbuffers/server.h"

#include <cstdint>
#include <flat_map>
#include <string>
#include <vector>

namespace sdlms {

// 摆摊摊位（095 CPersonalShopDlg 对应的房间数据）。
struct ServerPlayerShop {
  uint64_t owner = 0;             // 摊主 client_id
  uint32_t map_id = 0;
  std::u16string title;           // 店名
  // 货架：item_id + 剩余数量 + 单价。
  struct Goods {
    uint64_t item_id = 0;
    uint32_t num = 0;             // 剩余数量
    uint32_t sold = 0;            // 已售数量
    uint64_t price = 0;
  };
  std::vector<Goods> goods;
  std::vector<uint64_t> visitors; // 店内顾客（095：店内可见其它顾客）

  static constexpr size_t kMaxGoods = 16;   // 上限 16 种
  static constexpr uint32_t kMaxBuyNum = 200;
};

class ServerPlayerShopInstance {
 public:
  static inline std::flat_map<uint64_t, ServerPlayerShop> data_;  // owner → 摊

  // 摊主客户端消息（仅主机执行）。
  static void HandleClient(uint64_t client_id, fbs::ClientCharacterShopT& r);
  // 客户端收到的状态同步。
  static void HandleServer(uint64_t client_id, fbs::ServerCharacterShopT& r);
  // 帧循环：清理掉线摊主。
  static bool Run();
};

}  // namespace sdlms