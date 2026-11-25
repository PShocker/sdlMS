#pragma once

#include "src/client/game/game_character.h"
#include "src/client/system/ui/component/ui_component.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace sdlms {

// 人物摆摊（095 CPersonalShopDlg）。
// 窗口素材：UI/PlayerShop.img/Store（527x468）。
class PlayerShopUiSystem : public UiComponentHost<PlayerShopUiSystem> {
 public:
  enum class Mode {
    kIdle,      // 无窗口
    kSetup,     // 摊主的设置窗（上架/开张）
    kBrowsing,  // 顾客浏览别人的摊
    kSelling,   // 自己摊开张中
  };

  // 待售/货架条目。
  struct Goods {
    std::u16string item_id;  // 8 位道具 id
    uint32_t num = 0;         // 数量
    uint64_t price = 0;       // 单价
  };

  // 已售记录（095 SoldItems 列表）。
  struct SoldRecord {
    std::u16string item_id;
    uint32_t num = 0;
    uint64_t price = 0;
    std::u16string buyer;
  };

 private:
  static void RenderBackgrnd();
  static void RenderTitle();
  static void RenderOwner();      // 摊主名牌+立绘+金币
  static void RenderVisitor();    // 顾客位（浏览者自己）
  static void RenderGoodsList();  // 左侧货架（ItemEntry 风格）
  static void RenderChatPane();   // 设置=背包列表，其余=店内消息
  static void RenderFeeText();
  static void RenderSellEdit();   // ToSell 上架弹窗（数量+价格）
  static void RenderSoldList();   // SoldItems 已售弹窗
  static void RenderInputs();
  static bool EventUI(SDL_Event* event);

  static void EventButtonOpen();   // storeopen：开张（设置）
  static void EventButtonBuy();    // buy：购买（顾客）
  static void EventButtonClose();  // storeclose：关摊（开张中）
  static void EventButtonSold();   // sold：已售列表
  static void EventButtonLeave();  // leave/leave_host：离开
  static void EventButtonSellOk();     // ToSell OK：上架
  static void EventButtonSellCancel(); // ToSell Cancel

  static void SendStep(uint8_t step, uint64_t to_id = 0, uint8_t x = 0,
                       uint32_t y = 0);
  static void ApplyOpen();
  static void AddChat(const std::u16string& line);
  static void OpenSellEdit(const std::u16string& item_id, uint32_t max_num);

 public:
  static UIComponents& GetComponent();

  static inline Mode mode_ = Mode::kIdle;
  static inline SDL_FPoint pos_;
  static inline uint64_t owner_id_ = 0;       // 浏览对象的摊主
  static inline std::u16string stall_title_; // 店名
  static inline GameCharacter owner_char_{};  // 摊主立绘（浏览）
  static inline std::vector<Goods> goods_;    // 货架（设置=待售，浏览=对方货架）
  static inline std::vector<Goods> my_goods_; // 摊主已开张货架
  static inline int active_pkg_ = -1;         // 背包面板选中
  static inline int active_goods_ = -1;       // 货架选中
  static inline int goods_page_ = 0;
  static inline int pkg_page_ = 0;
  static inline uint64_t income_ = 0;         // 本次摆摊收入
  static inline std::vector<std::u16string> chat_log_;  // 店内消息
  static inline std::vector<SoldRecord> sold_log_;        // 已售记录
  static inline bool sold_open_ = false;      // 已售弹窗
  static inline int sold_page_ = 0;

  // ToSell 上架弹窗（点击背包物品后弹出）。
  static inline bool sell_edit_active_ = false;
  static inline std::u16string sell_item_id_;
  static inline uint32_t sell_max_num_ = 0;
  static inline TextInput sell_num_input_{};
  static inline TextInput sell_price_input_{};
  static inline int sell_edit_focus_ = 0;  // 0=数量 1=价格

  // 店内聊天输入（095 edit:chatinput (240,387)-(451,400)）。
  static inline TextInput chat_input_{};

  // 同图摊位招牌：client_id → 店名。
  static inline std::map<uint64_t, std::u16string> stalls_;

  static void OpenSetup();                    // 摊主：打开设置窗
  static void OpenBrowse(uint64_t owner);     // 顾客：请求进店
  static void Close();
  static void Toggle();

  static bool Render();
  static bool Event(SDL_Event* event);

  static SDL_FPoint LoadWh();
  static bool CursorIn();

  // 状态同步（ServerPlayerShopInstance::HandleServer 分流到这里）。
  static void HandleServer(uint64_t client_id, fbs::ServerCharacterShopT& r);
  // 招牌渲染：同图摊位（开摊玩家头顶招牌）。
  static bool RenderStalls();
};

}  // namespace sdlms