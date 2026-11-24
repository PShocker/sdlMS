#include "src/client/game_instance/friend_game_instance.h"

#include "src/client/game_instance/character_game_instance.h"

#include <algorithm>
#include <ranges>

namespace sdlms {

void FriendGameInstance::Load(const CharacterSaveData& cs) {
  friends_.clear();
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
  return std::erase_if(friends_, [id](const Friend& f) { return f.id == id; }) >
         0;
}

bool FriendGameInstance::IsOnline(uint64_t id) {
  for (auto it = CharacterGameInstance::others_.begin();
       it != CharacterGameInstance::others_.end(); ++it) {
    if (it->second.g_character.id == id) {
      return true;
    }
  }
  return false;
}

void FriendGameInstance::Clear() {
  friends_.clear();
}

}  // namespace sdlms