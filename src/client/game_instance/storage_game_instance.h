#pragma once

#include "src/client/game/game_save.h"
#include "src/client/game/game_storage.h"

#include <cstddef>

namespace sdlms {

// 账号级仓库数据。整个账号共用一个仓库，切换角色后仍然保留。
class StorageGameInstance {
 public:
  // 仓库格子数（超出窗口可视行数时通过滚动条翻页）。
  static constexpr size_t kSlotCount = 48;

  static inline GameStorage storage_;

  // 载入存档里的仓库内容，并按 kSlotCount 补齐空格子。
  static void Load(const GameSaveData& gs);

  // 把空格子补齐到 kSlotCount，保证窗口有可放入的槽位。
  static void EnsureSlots();
};

}  // namespace sdlms
