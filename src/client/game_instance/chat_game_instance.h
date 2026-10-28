#ifndef SDLMS_SRC_CLIENT_GAME_INSTANCE_CHAT_GAME_INSTANCE_H_
#define SDLMS_SRC_CLIENT_GAME_INSTANCE_CHAT_GAME_INSTANCE_H_
#include "src/client/game/game_chat.h"

#include <vector>
namespace sdlms {


class ChatGameInstance {
 public:
  static inline std::vector<GameChat> chats_;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_INSTANCE_CHAT_GAME_INSTANCE_H_
