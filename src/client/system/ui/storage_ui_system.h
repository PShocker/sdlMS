#pragma once

#include "src/client/game/game_item.h"
#include "src/client/game/game_npc.h"
#include "src/client/system/ui/component/ui_component.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace sdlms {

// 仓库（Trunk）窗口：左侧是仓库里的道具，右侧是角色背包，
// 选中道具后按 get／put 在两个容器之间搬运，按 sort 整理仓库。
// 由仓库 NPC（Npc.wz 里带 info/trunkPut 的 NPC）触发打开。
class StorageUiSystem : public UiComponentHost<StorageUiSystem> {
 private:
  static SDL_FPoint LoadWh();

  static void RenderBackgrnd();
  static void RenderTab();
  static void RenderNpc();
  static void RenderAvatar();
  static void RenderItems(std::vector<std::polymorphic<GameItem>>& items,
                          SDL_FPoint lt, uint16_t page,
                          const std::optional<uint16_t>& active);
  static void RenderMeso();
  static void RenderItemInfo();

  static bool AddItem(std::polymorphic<GameItem>& item,
                      uint64_t limit = UINT64_MAX);
  static void ClearItem(std::polymorphic<GameItem>& item);
  // 把第 tab 页第 index 个道具按 limit 件存入仓库（弹出数量框后调用）。
  static void PutItemCount(int tab, int index, uint64_t limit);
  // 把仓库第 slot_index 格按 limit 件取回背包（弹出数量框后调用）。
  static void TakeOutItem(int slot_index, uint64_t limit);

  static std::optional<uint32_t> LoadMouseIndex(SDL_FPoint lt, uint16_t page,
                                                size_t count);

  static void EventButtonGet();
  static void EventButtonPut();
  static void EventButtonSort();
  static void EventButtonInCoin();   // 存入 1000 金币
  static void EventButtonOutCoin();  // 取出 1000 金币
  static void EventButtonExit();
  static bool EventItem(SDL_Event* event);
  static bool EventUI(SDL_Event* event);
  static void EventClose();

  static inline std::array<std::optional<uint16_t>, 2> active_item_;
  static inline int active_tab_ = 0;
  static inline std::array<uint16_t, 2> page_;
  // 正在拖动的滚动条：0 = 仓库侧，1 = 背包侧，-1 = 没有
  static inline int drag_scroll_ = -1;

 public:
  static UIComponents& GetComponent();

  static inline std::u16string npc_id_;
  static inline std::optional<GameNpc> npc_;
  static inline SDL_FPoint pos_;

  static bool CursorIn();

  static void Toggle();

  static void Open();
  static void Close();
  static bool Render();
  // 滚动条拖动 / 滚轮翻页用：鼠标所在的滚动条或滚动区域（0=仓库，1=背包）
  static int ScrollSideAt(SDL_FPoint mouse);
  static int WheelSideAt(SDL_FPoint mouse);
  static void ApplyScrollDrag(int side);

  static bool Event(SDL_Event* event);
};

}  // namespace sdlms
