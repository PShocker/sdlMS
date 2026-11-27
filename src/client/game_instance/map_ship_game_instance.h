#pragma once

#include <SDL3/SDL_rect.h>
#include <wz/Node.h>

#include <cstdint>
#include <string>

namespace sdlms {

// 095 CShip：地图 shipObj 节点。
//   shipKind 0 = 行驶中的船（从 x0 移动到 x，tMove 秒）
//   shipKind 1 = 停泊的船（在 x/y 附近随机漂移 ±50/±100）
struct GameShip {
  bool exists = false;
  int kind = 0;
  int t_move = 0;
  int x = 0;
  int y = 0;
  int x0 = 0;
  SDL_FPoint pos{};
  SDL_FPoint from{};
  SDL_FPoint to{};
  SDL_FPoint origin{};
  wz::Node* sprite = nullptr;
  uint64_t move_start = 0;
  uint64_t move_duration = 0;
  uint64_t next_move = 0;
};

class MapShipGameInstance {
 public:
  static void Load(uint32_t map_id);
  static void Clear();
  static bool Run();
  static bool Render();

 private:
  static inline GameShip ship_;
};

}  // namespace sdlms
