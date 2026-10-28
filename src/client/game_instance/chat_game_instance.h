#pragma once
#include "src/client/game/game_chat.h"
#include <vector>
namespace sdlms {


class ChatGameInstance {
public:
  static inline std::vector<GameChat> chats_;
};

}  // namespace sdlms
