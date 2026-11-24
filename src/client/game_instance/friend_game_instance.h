#pragma once

#include "src/client/game/game_save.h"

#include <cstdint>
#include <string>
#include <vector>

namespace sdlms {

// 好友列表：用人物唯一 id 作为好友标识，名字用于显示。
class FriendGameInstance {
 public:
  struct Friend {
    uint64_t id = 0;
    std::u16string name;
  };

  static inline std::vector<Friend> friends_;

  static void Load(const CharacterSaveData& cs);
  static void Save(CharacterSaveData& cs);
  // 用人物 id 添加好友；id 为 0、自己、或已存在时返回 false。
  static bool Add(uint64_t id, const std::u16string& name);
  static bool IsFriend(uint64_t id);
  static bool Remove(uint64_t id);
  // 好友 id 是否在线（在当前地图的其它角色里查找）。
  static bool IsOnline(uint64_t id);
  static void Clear();
};

}  // namespace sdlms