#pragma once

#include <cstdint>
#include <string>

namespace sdlms {

enum class GameChatEnum {
  kMap,
  kWhisper,
  kParty,
  kGuild,
};

struct GameChat {
  uint64_t client_id;
  GameChatEnum type;
  std::u16string owner;
  std::u16string text;

  uint64_t time;
};

}  // namespace sdlms
