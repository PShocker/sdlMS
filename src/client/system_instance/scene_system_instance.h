#pragma once

#include "src/client/game/game_character.h"

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <functional>
#include <optional>
#include <string>

namespace sdlms {

class SceneSystemInstance {
 public:
  static inline uint32_t map_id_;
  static inline uint32_t prepare_map_id_;
  static inline std::optional<SDL_FPoint> prepare_pos_;
  static void Enter(uint32_t map_id);
  static void EnterFade();
  static void EnterPrepare(uint32_t map_id, const std::u16string& pn, int i);
  static void EnterQuit();
  static void QuitPrepare();
  static void EnterCashShop();
  static void CashShopPrepare();
  // 从现金商店返回地图：发 ClientScene{fade=false} 触发正常地图重进握手，
  // 把客户端重新登记进服务端场景（怪物/其它玩家恢复），并回到指定位置。
  static void EnterCashShopReturn(uint32_t map_id, SDL_FPoint pos);

  // 场景准备完成（地图数据加载完、角色已就位）时执行一次，然后自动清空。
  static inline std::function<void()> on_prepare_cb_;

 private:
  static SDL_FPoint LoadPreparePos(const std::u16string& pn, uint8_t index);
  static void EnterPrepare();
  static bool RenderGame();
};

}  // namespace sdlms
