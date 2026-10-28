#pragma once
#include <cstdint>
#include <string>

enum class GameChatEnum {
  map,
};
struct GameChat {
  uint64_t client_id;
  GameChatEnum type;
  std::u16string owner;
  std::u16string text;

  uint64_t time;
};