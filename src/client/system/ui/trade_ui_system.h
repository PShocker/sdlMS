#pragma once

#include "src/client/game/game_character.h"
#include "src/client/system/ui/component/ui_component.h"
#include "src/common/flatbuffers/common.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <vector>

namespace sdlms {

class TradeUiSystem : public UiComponentHost<TradeUiSystem> {
 private:
  static void RenderBackgrnd();
  static void RenderGrids();
  static void RenderInventoryStrip();

  static void EventButtonClose();
  static void EventButtonTrade();
  static void EventButtonReset();
  static void EventButtonCoin();
  static void EventButtonNoop();
  static bool EventUI(SDL_Event* event);

 public:
  struct TradeOffer {
    uint8_t tab = 0;
    uint32_t index = 0;
  };

  static UIComponents& GetComponent();
  static inline SDL_FPoint pos_;
  static inline GameCharacter other_;
  static inline uint64_t other_id_ = 0;
  static inline uint64_t my_meso_ = 0;
  static inline uint64_t other_meso_ = 0;
  // 自己放入交易的物品（包裹 tab + 槽位）
  static inline std::vector<TradeOffer> my_offers_;
  // 对方放入交易的物品（服务端转发过来的 ItemUnion 列表）
  static inline std::vector<fbs::ItemUnionUnion> other_payload_;

  // 放入/取出一件自己的物品到交易（由底部背包条点击调用）。
  static void OfferItem(uint8_t tab, uint32_t index);
  // 成交结算：移除自己放出的物品，加入对方给出的物品。
  static void ApplyTrade();

  static bool CursorIn();

  static void Toggle();
  static SDL_FPoint LoadWh();

  static void Open();
  static void Close();
  static bool Render();
  static bool Event(SDL_Event* event);
};

}  // namespace sdlms