#pragma once

#include "src/client/game/game_save.h"

#include <cstdint>
#include <flat_set>
#include <string>
#include <vector>

namespace sdlms {

// 好友列表：用人物唯一 id（高32位=创建时间戳，低32位=随机数）作为好友标识，
// 名字只用于显示；好友上线/下线由服务端广播（ServerFriendOnline）驱动。
class FriendGameInstance {
 public:
  struct Friend {
    uint64_t id = 0;
    std::u16string name;
  };

  static inline std::vector<Friend> friends_;
  // 当前在线的人物 id（服务端广播维护，含非好友：方便"先加好友后查在线"）。
  static inline std::flat_set<uint64_t> online_;

  static void Load(const CharacterSaveData& cs);
  static void Save(CharacterSaveData& cs);
  // 用人物 id 添加好友；id 为 0、自己、或已存在时返回 false。
  static bool Add(uint64_t id, const std::u16string& name);
  static bool IsFriend(uint64_t id);
  static bool Remove(uint64_t id);
  // 好友 id 是否在线（查服务端广播维护的在线集合）。
  static bool IsOnline(uint64_t id);

  // 服务端 ServerFriendOnline 的处理：
  // 维护在线集合、同步好友改名，返回 true 表示"好友首次上线"需要弹提示。
  static bool SetOnlineState(uint64_t id, const std::u16string& name,
                             bool online);

  static void Clear();
};

}  // namespace sdlms