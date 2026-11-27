#include "src/client/game_instance/friend_game_instance.h"

#include "src/client/game_instance/character_game_instance.h"

#include <algorithm>
#include <ranges>

namespace sdlms {

void FriendGameInstance::Load(const CharacterSaveData& cs) {
  friends_.clear();
  online_.clear();
  for (const auto& f : cs.friends) {
    if (f.id == 0) {
      continue;
    }
    friends_.push_back({f.id, f.name});
  }
}

void FriendGameInstance::Save(CharacterSaveData& cs) {
  cs.friends.clear();
  for (const auto& f : friends_) {
    cs.friends.push_back({f.id, f.name});
  }
}

bool FriendGameInstance::Add(uint64_t id, const std::u16string& name) {
  if (id == 0 || id == CharacterGameInstance::self_.id) {
    return false;
  }
  if (IsFriend(id)) {
    return false;
  }
  friends_.push_back({id, name});
  return true;
}

bool FriendGameInstance::IsFriend(uint64_t id) {
  return std::ranges::any_of(friends_,
                             [id](const Friend& f) { return f.id == id; });
}

bool FriendGameInstance::Remove(uint64_t id) {
  online_.erase(id);
  return std::erase_if(friends_, [id](const Friend& f) { return f.id == id; }) >
         0;
}

bool FriendGameInstance::IsOnline(uint64_t id) { return online_.contains(id); }

bool FriendGameInstance::SetOnlineState(uint64_t id, const std::u16string& name,
                                        bool online) {
  if (id == 0) {
    return false;
  }
  if (!online) {
    online_.erase(id);
    return false;
  }
  const bool was_online = online_.contains(id);
  online_.insert(id);

  // 好友改名：用服务端广播的最新名字刷新本地显示。
  if (!name.empty()) {
    for (auto& f : friends_) {
      if (f.id == id && f.name != name) {
        f.name = name;
      }
    }
  }
  return !was_online && IsFriend(id);
}

void FriendGameInstance::Clear() {
  friends_.clear();
  online_.clear();
}

}  // namespace sdlms