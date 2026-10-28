#ifndef SDLMS_SRC_CLIENT_GAME_GAME_CHAT_H_
#define SDLMS_SRC_CLIENT_GAME_GAME_CHAT_H_
#include <cstdint>
#include <string>
namespace sdlms {


enum class GameChatEnum {
  kMap,
};
struct GameChat {
  uint64_t client_id;
  GameChatEnum type;
  std::u16string owner;
  std::u16string text;

  uint64_t time;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_GAME_CHAT_H_
