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

  // 095 HiredShop：执照道具 0503000x + 固定摆位；0=普通摆摊。
  uint32_t hired_item = 0;
  int32_t x = 0;
  int32_t y = 0;
  // 雇佣商店离线期间累计的销售收入（摊主回来后可领取）。
  uint64_t income = 0;

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
  // 进图时补发本图已有摄位的开摄广播（step 5）。
  static void SendStallsTo(uint64_t client_id, uint32_t map_id);
  // 帧循环：清理掉线摊主。
  static bool Run();
};

}  // namespace sdlms